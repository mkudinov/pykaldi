class Transcriber(object):
    def __init__(self, word_table, unknown_word_code, encoding='default'):
        self._word_table = word_table
        assert(unknown_word_code in self._word_table)
        self._unk_code = self._word_table[unknown_word_code]
        self._encoding = encoding

    def transcribe(self, phrase):
        if self._encoding != 'default':
            phrase = phrase.encode(self._encoding)
        return [self._word_table[word] if word in self._word_table else self._unk_code for word in phrase.split(' ') ]


def load_word_table(path_to_word_table):
    word_table = {}
    for line in open(path_to_word_table, 'r'):
        word_code = line.split(' ')
        word = word_code[0]
        code = int(word_code[1])
        word_table[word] = code
    return word_table

def load_phone_table(path_to_phones_table):
    phone_table = []
    for line in open(path_to_phones_table, 'r'):
        phone, phone_id = line.split(' ')
        phone_table.append(phone)
    if len(phone_table) == 0:
        raise RuntimeError('Empty phone table read from file {}'.format(path_to_phones_table))
    return phone_table