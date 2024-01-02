# Zehl's Enigma Machine (ZIGMA)
This program is a nifty little tool written for the consummate spy or paranoid hacker who wants to
transmit "secret" information across open, text-based networks (like community boards, forums, SMS, or
even social media). It has also proven to be a relatively reliable way to store small files with a
moderate degree of security. It hopes to be extensible, maintainable, and ultimately portable.

## Functionality
The program can read from a file (or standard input, if omitted) and can write to a file (or standard
output, if omitted). The format of the input and output can be binary, base-16, or base-64 encoded. The
program will prompt for a passphrase if a key file is omitted.

## The ZIGMA Cipher
The stream cipher is a state machine with a permutation vector of 256 bytes indexed with feedback from
each plaintext and cipher-text character. The vector is "perturbed" first with a key, and then each
byte is fed into the machine. The cipher function is nonlinear and has several moving indices. The
original cipher appeared somewhere as an experiment on *sci.crypt.research* in the late 90s. The source
has been maintained by the author in one form or another since 1999 (ported to C, C++, and Delphi).

## A few Considerations
There are some practical limitations to the algorithm and the application.
  * A key must be no more than **256 characters** in length (2048 bits).
    * **Strength in numbers**: The cipher is most secure when all 256 bytes are used.
  * Encoding the cipher-text as base 16 or base 64 increases the overall size by at least 2:1 and 4:3,
   respectively, plus additional space for newlines in the output column.
  * Checksums and error-correction data are not present in the output stream.
    * The user is not protected from transposition or typographical errors in the transmission of the
      message. Care must be taken to ensure that the message is reassembled correctly before decoding.
    * The program doesn't handle or generate "incorrect-key" errors.

## How to Use

To encode `README.md` from base `256` and output to `README.md.crypt` in base `64`
~~~
$ zigma encode in=README.md out=README.md.crypt in.fmt=256 out.fmt=64
~~~

To decode `README.md.crypt` from base `64` and output to `README.md` in base `256`
~~~
$ zigma decode in=README.md.crypt out=README.md in.fmt=64 out.fmt=256
~~~
