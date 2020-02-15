# Description
A simple offline diceware passphrase generator.
Similar to [this site](https://www.rempe.us/diceware/#eff), but runs offline.

# Wordlist sources
Diceware wordlist: [link](http://world.std.com/~reinhold/diceware.wordlist.asc)
EFF wordlist: [link](https://www.eff.org/files/2016/07/18/eff_large_wordlist.txt)

#Usage:

    python gen.py --wordlist=<eff/diceware> --num=<number of words> --times=<number of phrases>

For full list of command line options type `python gen.py --help`
