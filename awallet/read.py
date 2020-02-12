# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


import zlib
import gzip
import xml.dom.minidom
import getpass

from Crypto.Cipher import AES
from Crypto.Cipher import Blowfish
from Crypto.Hash import SHA256

from absl import app
from absl import flags
from absl import logging

FLAGS = flags.FLAGS

flags.DEFINE_string('file', None, 'Encrypted data.crypt file')
flags.DEFINE_boolean('match_only', False,
                     'Just check for password match only.')
flags.DEFINE_string('password', None, '(Optional) Password.')

flags.mark_flag_as_required('file')


def read_crypt_file():
    """Processing begins here."""
    with open(FLAGS.file, 'rb') as f:
        data = f.read()

    salt = data[:64]

    if not FLAGS.password:
        mpass = getpass.getpass().encode()
    else:
        mpass = FLAGS.password.encode()

    cur_hash = salt + mpass
    for x in range(1000):
        sha = SHA256.new(cur_hash)
        cur_hash = sha.digest()

    key256 = sha.digest()
    key192 = sha.digest()[:24]
    key128 = sha.digest()[:16]

    all_keys = [key256, key192, key128]

    blowfish_modes = [
        Blowfish.MODE_CBC,
        Blowfish.MODE_CFB,
        Blowfish.MODE_OFB,
        Blowfish.MODE_ECB,
    ]
    aes_modes = [
        AES.MODE_CBC,
        AES.MODE_CFB,
        AES.MODE_OFB,
        AES.MODE_ECB,
    ]

    main_data = data[64:]

    # Try Blowfish
    for cur_key in all_keys:
        for cur_mode in blowfish_modes:
            cipher = Blowfish.new(cur_key, cur_mode)
            decrypted_data = cipher.decrypt(main_data)
            check_val = decrypted_data[:4]
            if check_val == b'TRUE':
                if FLAGS.match_only:
                    logging.info('Password matched.')
                else:
                    show_data(decrypted_data)
                return

    # Try AES
    for cur_key in all_keys:
        for cur_mode in aes_modes:
            cipher = AES.new(cur_key, cur_mode)
            decrypted_data = cipher.decrypt(main_data)
            check_val = decrypted_data[:4]
            if check_val == b'TRUE':
                if FLAGS.match_only:
                    logging.info('Password matched.')
                else:
                    show_data(decrypted_data)
                return


def show_data(decrypted_data):
    version_idx = slice(4, 6)
    crc32_idx = slice(8, 12)
    data_size_idx = slice(12, 16)
    actual_data_start = 16

    # Format version.
    # logging.info("Version: %d", int.from_bytes(decrypted_data[version_idx], 'big'))

    # Size of data
    data_size = int.from_bytes(decrypted_data[data_size_idx], 'big')
    actual_data_end = actual_data_start + data_size
    actual_data = decrypted_data[actual_data_start:actual_data_end]

    # CRC-32
    crc_32 = int.from_bytes(decrypted_data[crc32_idx], 'big')
    actual_crc_32 = zlib.crc32(actual_data)
    if crc_32 != actual_crc_32:
        logging.error('CRC 32 doesn\'t match. Exiting.')
        return

    xml_data = gzip.decompress(actual_data)
    dom = xml.dom.minidom.parseString(xml_data)
    print(dom.toprettyxml())


def main(argv):
    del argv
    read_crypt_file()


if __name__ == '__main__':
    app.run(main)

