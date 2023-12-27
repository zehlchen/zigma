# Zehl's Enigma Machine
The **ZIGMA** (*ZEHL + ENIGMA*) program allows users to create cryptograms suitable for use on public 
networks and platforms such as social media, popular chat and forum software, email and SMS messages, 
and even scannable QR-codes. **ZIGMA** attempts to provide enough security for those with clinical
paranoia and has been written and designed to be an extensible, maintainable, and portable software
solution. **ZIGMA** does not embed metadata in its output stream, so the validity of a key can only
be tested contextually.

## Caveat Emptor
There are some limitations to the ZIGMA cipher and its command-line interface.
 * A key is a secret combination of up to 256 bytes (2048 bits) and is only known to those with whom
   the key has been pre-shared. Only the exact key used to encrypt can be used to decrypt.
 * The cipher is non-linear and relies on feedback from the key and the contents of the data being
   encoded as well as other indices. 
 * The contents of the cipher-text (the encrypted data) are visible to anyone, and is subject to
   analysis by the general public.
 * Base16 and Base64 encoding will increase the size of the output by 2:1 and 4:3 respectively.
 * There is no way to verify if a given key matches a given cryptogram without looking at the
   decrypted data. The encrypted message does not include a checksum or error-correction data, so
   there is no protection against data loss due to bit rot or bitwise transposition.
 * The original, unencrypted message is not known to an attacker, either in part or in whole.
 * The computational complexity of the algorithm is such that direct attack is nontrivial and
   prohibitive.

## How to Use
~~~
$ zigma <MODE> [OPERAND (KEY=VALUE)...]
~~~
`MODE` must be one of the following:
 * `encode` to encrypt
 * `decode` to decrypt
 * `check` to generate a checksum.

and `OPERAND` may be a combination of the following:
 * `input=FILE` (omit to read from standard input)
 * `output=FILE` (omit to write to standard output)
 * `key=FILE` (omit to prompt for a passphrase)
 * `format=BASE` (default is `64`)