# Zehl's Enigma Machine 2.0
ZIGMA can be used to send secure messages over insecure, text-based networks. The software uses a combination of
feedback from a key file or passphrase, the raw input (the message being transmitted), and a 256-byte permutation 
vector in a way that is analogous to the functioning of the Enigma machine as developed nearly a century ago.

The program reads from standard input or a specified file and writes to standard output or a specified file. The
user will provide a key either as a passphrase (a long password) or a key file. A key may be up to 256 bytes long
and is assumed to be secretly pre-shared. Only with this key can the cryptogram be recovered.

The cipher is byte-oriented, and can be used with formatted input/output or directly as binary
 * `16` (hexadecimal representation) - useful for mathematical analysis (2:1 size penalty)
 * `64` (plain text) - this is the default, and is intended for use on text-based networks (4:3 size penalty)
 * `256` (unfiltered binary) - this can be stored in any byte-oriented format (no size penalty)

