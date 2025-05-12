# DES Image Steganography

A concise C++ program that combines classic DES encryption with image steganography using OpenCV.

## Features

* **Full DES Implementation**: Key scheduling, initial/final permutations, Feistel rounds, S‑boxes, and P‑permutation.
* **Block & Stream API**: `DES_enc_block`, `DES_dec_block`, `DES_enc`, `DES_dec`.
* **Image I/O**: Read/write color (RGB) and grayscale images.
* **Embedding**: Encrypt and hide a grayscale image’s bitstream inside an RGB cover image.
* **Extraction**: Recover and reconstruct the hidden grayscale payload.

## Requirements

* C++17 or later
* OpenCV (>=4.0)

