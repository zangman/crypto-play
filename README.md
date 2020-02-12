# Crypto Play

## About
This is a weekend project where I tried to decrypt a couple of secure wallets
that I use on a regular basis. It has been fun to work with and I did learn a
lot. A list of resources that helped me are at the bottom.

## Disclaimers

 * **Incomplete**: The code here is incomplete. It doesn't fully implement every possible scenario for every wallet (For example, keepass is missing the Argon2 KDF). It was just a learning experience and perhaps someone else will find it useful to learn something.

 * **Not Secure**: Nothing in this repository is secure. _Please don't use this with your regular encrypted datastores._ Everything is printed to `stdout` and anyone can see it or it could be stored as plain text in your machine.

 * **Not for cracking**: The code here is woefully un-optimized and slow. This will only be good for cracking a password that's 3 characters long.

 * **Use other tools/libs**: There are several better documented and more well maintained tools out there (links at the bottom). Use those instead.


## Current Decrypters
 * [aWallet Password Manager](awallet.org)
 * [Keepass 2 Password Manager](keepass.info)



## Usage

The code here was written with Python 3.8.1. Steps to use this:

1. Clone the project.
1. `cd` into the folder of tool of your choice.
1. Create a new python virtual environment:
        python -m venv v
1. Activate the virtual environment:

        source v/bin/activate

1. Install dependencies:

        pip install -r requirements.txt

1. Run the following to see the usage:

        python read.py --help


## Resources

### aWallet

The aWallet author has been quite open in sharing the encryption details. The clear and detailed instructions are available on [this page](http://www.awallet.org/documentation/technical-documentation). That alone is sufficient.

### Keepass 2

Keepass was a bit of a hassle mainly because they don't have a clear outline of the steps to undertake to encrypt/decrypt. Which is really weird for an open source project. I had to download the source code (which is written in C#), set it up in Visual Studio and Debug and step through it over a few days to finally understand what was going on. But even after that, I got stumped on the structure of the final decrypted block and that's where [this page](https://gist.github.com/lgg/e6ccc6e212d18dd2ecd8a8c116fb1e45) came in super helpful.

### Other resources

If you're a programmer new to Cryptography, I highly recommend reading [Crypto101](https://www.crypto101.io/). It is not complete, but the sections on Block Ciphers and Stream Ciphers are more than sufficient for this project.

## Contribution

If you find any bugs or code improvements, feel free to send a pull request. Refer to the `contributing.md` file for the Contributer License Agreement.
