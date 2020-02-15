from absl import app
from absl import flags
from absl import logging

from Crypto.Random import random

FLAGS = flags.FLAGS

flags.DEFINE_integer('num', 6, 'Number of words to generate.')
flags.DEFINE_string('wordlist', 'eff', 'wordlist to use (eff/diceware)')
flags.DEFINE_integer('times', 10, 'Number of times to run.')


def get_wordlist():
    if FLAGS.wordlist == 'eff':
        file_name = 'eff_large_wordlist.txt'
    elif FLAGS.wordlist == 'diceware':
        file_name = 'diceware.wordlist.asc'
    with open(file_name) as f:
        data = f.read().split('\n')

    data.remove('')
    wordlist = {int(a.split('\t')[0]): a.split('\t')[1] for a in data}
    return wordlist


def generate_5_digits():
    num = 0
    for i in range(5):
        r = random.randint(1, 6)
        num += r
        num *= 10
    num /= 10
    return int(num)


def get_words(wordlist, num):
    words = []
    length = 0
    for i in range(num):
        r = generate_5_digits()
        new_word = wordlist[r]
        length += len(new_word)
        words.append(new_word)

    return words, length


def main(argv):
    del argv
    num = FLAGS.num
    times = FLAGS.times
    for i in range(0, times):
        wordlist = get_wordlist()
        phrase, length = get_words(wordlist, num)
        print(f'len: {length} ' + str(phrase))


if __name__ == '__main__':
    app.run(main)
