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


import gzip
import getpass
import xml.dom.minidom

from absl import app
from absl import flags
from absl import logging

from bs4 import BeautifulSoup

from base64 import b64decode

from Crypto.Cipher import AES
from Crypto.Cipher import Salsa20
from Crypto.Hash import SHA256
from Crypto.Hash import SHA512
from Crypto.Hash import MD5
from Crypto.Util import Padding

from typing import Tuple

FLAGS = flags.FLAGS

flags.DEFINE_string('file', None, 'Keepass db file.')
flags.DEFINE_boolean('metadata', False, 'Print kbdx metadata.')
flags.DEFINE_string('out', None, 'Output xml file.')

flags.mark_flag_as_required('file')

_SIG1_OFFSET = 4
_SIG2_OFFSET = 4
_VERSION_OFFSET = 4
_BIT_FIELD_ID_OFFSET = 1
_BIT_FIELD_SIZE_OFFSET = 2
_STREAM_START_BYTES_OFFSET = 32

_FIELDS = {
    0: 'EndOfHeader',
    1: 'Comment',
    2: 'CipherID',
    3: 'CompressionFlags',
    4: 'MasterSeed',
    5: 'TransformSeed',
    6: 'TransformRounds',
    7: 'EncryptionIV',
    8: 'InnerRandomStreamKey',
    9: 'StreamStartBytes',
    10: 'InnerRandomStreamID',
    11: 'KdfParameters',
    12: 'PublicCustomData',
}

_AES_UUID = '0xff5afc6a210558be504371bfe6f2c131'
_GZIP_ID = '0x1'

# For unencrypted data.
_BLOCK_ID_OFFSET = 4
_BLOCK_HASH_OFFSET = 32
_BLOCK_DATA_SIZE_OFFSET = 4

# For protected passwords.
_SALSA_IV = bytes.fromhex('e830094b97205d2a')


def _get_int(byte_val: bytes) -> int:
    return int.from_bytes(byte_val, 'little')


def _read_data(data: bytes, index: int, offset: int) -> Tuple[bytes, int]:
    if offset == -1:
        val = data[index:]
        new_offset = -1
    else:
        val = data[index:index + offset]
        new_offset = index + offset
    return val, new_offset


def read_keepass(file_path: str, user_pass: bytes):
    meta = FLAGS.metadata
    master_seed = None
    transform_seed = None
    transform_rounds = None
    inner_random_stream_key = None
    iv = None
    stream_start_bytes = None

    with open(file_path, 'rb') as f:
        encrypted_data = f.read()

    offset = 0

    sig1, offset = _read_data(encrypted_data, offset, _SIG1_OFFSET)
    sig2, offset = _read_data(encrypted_data, offset, _SIG2_OFFSET)
    version, offset = _read_data(encrypted_data, offset, _VERSION_OFFSET)

    if meta:
        logging.info('Signature 1: %s', _get_hex(sig1))
        logging.info('Signature 2: %s', _get_hex(sig2))
        logging.info('Version: %s', _get_hex(version))

    while True:
        bit_field_id, offset = _read_data(encrypted_data, offset,
                                          _BIT_FIELD_ID_OFFSET)
        bit_field_id_val = _get_int(bit_field_id)

        bit_field_size, offset = _read_data(encrypted_data, offset,
                                            _BIT_FIELD_SIZE_OFFSET)
        bit_field_size_val = _get_int(bit_field_size)

        header_val, offset = _read_data(encrypted_data, offset,
                                        bit_field_size_val)

        if bit_field_id_val == 0:
            break

        field = _FIELDS.get(bit_field_id_val)

        if meta:
            logging.info('Id: %s', field)
            logging.info('Size: %d', bit_field_size_val)
            logging.info('Val: %s', header_val.hex())

        if field == 'MasterSeed':
            master_seed = header_val
        elif field == 'TransformSeed':
            transform_seed = header_val
        elif field == 'TransformRounds':
            transform_rounds = _get_int(header_val)
        elif field == 'EncryptionIV':
            iv = header_val
        elif field == 'InnerRandomStreamKey':
            inner_random_stream_key = header_val
        elif field == 'StreamStartBytes':
            stream_start_bytes = header_val

    # Hash user password twice.
    one_pass = SHA256.new(user_pass).digest()
    two_pass = SHA256.new(one_pass).digest()

    a = AES.new(transform_seed, AES.MODE_ECB)

    # Encrypt it as many times as transform_rounds.
    user_pass_aes = two_pass
    for i in range(transform_rounds):
        user_pass_aes = a.encrypt(user_pass_aes)

    # Hash once more.
    user_pass_hash = SHA256.new(user_pass_aes).digest()

    # Master Seed + above value.
    in_key = master_seed + user_pass_hash
    out_key = SHA256.new(in_key).digest()

    # Needed for Keepass 4+ (TODO)
    # out_key2 = in_key + bytes.fromhex('01')
    # out_key_64 = SHA512.new(out_key2).digest()
    # logging.info('Out key 64: %s', out_key_64.hex())

    decrypter = AES.new(out_key, AES.MODE_CBC, iv=iv)

    stream_start_bytes_data_encrypted, offset = _read_data(
        encrypted_data, offset, _STREAM_START_BYTES_OFFSET)

    stream_start_bytes_data = decrypter.decrypt(
        stream_start_bytes_data_encrypted)

    if stream_start_bytes != stream_start_bytes_data:
        logging.error('Key isn\'t working')
        return

    main_data, _ = _read_data(encrypted_data, offset, -1)
    xml_data = _get_xml(decrypter.decrypt(main_data))
    xml_data_decrypted = _decrypt_protected(xml_data, inner_random_stream_key)

    if FLAGS.out:
        with open(FLAGS.out, 'w') as f:
            f.write(xml_data_decrypted)
    else:
        print(xml_data_decrypted)


def _get_xml(data: bytes) -> str:
    offset = 0
    blocks = b''

    while offset < len(data):
        if FLAGS.metadata:
            logging.info('Offset: %d, data: %d', offset, len(data))
        block_id, offset = _read_data(data, offset, _BLOCK_ID_OFFSET)
        block_id_int = _get_int(block_id)

        if FLAGS.metadata:
            logging.info('Got block id: %s', block_id_int)

        block_hash, offset = _read_data(data, offset, _BLOCK_HASH_OFFSET)
        block_data_size, offset = _read_data(data, offset,
                                             _BLOCK_DATA_SIZE_OFFSET)
        block_data, offset = _read_data(data, offset,
                                        _get_int(block_data_size))
        if not block_data or _get_int(block_hash) == 0:
            break

        data_hash = SHA256.new(block_data).digest()

        if block_hash != data_hash:
            logging.error('Expecting: %s', block_hash.hex())
            logging.error('Got:       %s', data_hash.hex())
            raise ValueError('Whoops, hash doesn\'t match')

        blocks += block_data

    xml_data = gzip.decompress(blocks)
    return xml_data.decode()


def _decrypt_protected(xml_data: bytes, key: bytes) -> str:
    '''Decrypts all the passwords in the xml.'''
    salsa_key = SHA256.new(key).digest()
    # Stream cipher needs to be created once and used sequentially
    # on each element to decrypt. Do not create a new instance
    # for each element.
    salsa_cipher = Salsa20.new(salsa_key, _SALSA_IV)
    soup = BeautifulSoup(xml_data, 'xml')
    protected_elements = soup.find_all(attrs={'Protected': 'True'})
    for ele in protected_elements:
        text_b64decoded = b64decode(ele.text)
        ele.string = salsa_cipher.decrypt(text_b64decoded).decode()
    return soup.prettify()


def main(argv):
    del argv
    user_pass = getpass.getpass().encode()
    read_keepass(FLAGS.file, user_pass)


if __name__ == '__main__':
    app.run(main)
