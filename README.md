# Huffman Compression
A minimal huffman compression program using C++ language.

## Theory
Huffman compression is based on huffman coding technique. The huffman coding creates an optimal binary tree that is constructed based on frequency of an item/character in a file.


Let's take an example of a file or string containing data like "AAABBCAAADDFFAAAADCCCDAADDDAAACGAAACACA"
```
character     frequency
    A             20
    B              2
    C              7
    D              7
    F              2
    G              1
```
The above following data will generate a binary tree starting from the characters having the lowest frequency, and construct till we use all the characters as follows:

```
Pass 1:
(A, 20) (C, 7) (D, 7) (B, 2) (F, 2) (G, 1)                      (A, 20) (C, 7) (D, 7) (**, 3)  (B, 2)   
                                                   =>                                  /   \
                                                                                      /     \
                                                                                   (F, 2) (G, 1)

Pass 2:
(A, 20) (C, 7) (D, 7) (**, 3)  (B, 2)                           (A, 20) (C, 7) (D, 7) (**, 5)
                       /   \                                                           /   \
                      /     \                                                         /     \
                   (F, 2) (G, 1)                   =>                              (**, 3) (B, 2)
                                                                                    /   \
                                                                                   /     \
                                                                                (F, 2) (G, 1)

Pass 3:
(A, 20) (C, 7) (D, 7) (**, 5)                                    (A, 20) (C, 7) (**, 12)
                       /   \                                                     /   \
                      /     \                                                   /     \
                   (**, 3) (B, 2)                  =>                        (D, 7) (**, 5)
                    /   \                                                            /   \
                   /     \                                                          /     \
                 (F, 2) (G, 1)                                                   (**, 3) (B, 2)
                                                                                  /   \
                                                                                 /     \
                                                                              (F, 2) (G, 1)
                                                                              
Pass 4:
(A, 20) (**, 12) (C, 7)                                           (A, 20) (**, 19)
          /   \                                                             /   \
         /     \                                                           /     \
      (D, 7) (**, 5)                                                   (**, 12) (C, 7)
              /   \                                                      /   \
             /     \                               =>                   /     \
          (**, 3) (B, 2)                                            (D, 7) (**, 5)
           /   \                                                            /   \
          /     \                                                          /     \
       (F, 2) (G, 1)                                                    (**, 3) (B, 2)
                                                                         /   \
                                                                        /     \
                                                                     (F, 2) (G, 1)
                                                                     
Pass 5 (Final), with huffman code:

Left Branch denoting 0 and right 1

(A, 20) (**, 19)                                                         (**, 39)
          /   \                                                           /   \
         /     \                                                      (0)/     \(1)
     (**, 12) (C, 7)                                          [0] <= (A, 20) (**, 19)
       /   \                                                                 /   \
      /     \                                                            (0)/     \(1)
  (D, 7) (**, 5)                                    =>                  (**, 12) (C, 7) => [11]
          /   \                                                          /   \
         /     \                                                     (0)/     \(1)
     (**, 3) (B, 2)                                        [100] <= (D, 7) (**, 5)
      /   \                                                                 /   \
     /     \                                                               /     \(1)
  (F, 2) (G, 1)                                                        (**, 3) (B, 2) => [1011]
                                                                        /   \
                                                                    (0)/     \(1)
                                                         [10100] <= (F, 2) (G, 1) => [10101]
                                                         
Final Huffman Codes:
character     frequency       Huffman Codes       Actual Binary
    A             20                  0             01000001
    B              2               1011             01000010
    C              7                 11             01000011
    D              7                100             01000100
    F              2              10100             01000110
    G              1              10101             01000111
__________________________________________________________________
                  39                 75                  312
```
Hence the resultant data is stored as binary written as '0001011101100010010010100101000000100111111100001001001000001110101000110110*00000*', which has 75 bits
plus 5 digits appended to round off the remaining bits while storing in the file.


## Executing program

```
g++ huffman-compression.cpp
[a.exe | ./a.out] -c|-dc [filename_to_be_compressed] 
(The order must be same: first: option to compress/decompress and then second: filename)
```
The file to be compressed will generate a file with extension '.abiz', which is the compressed version of the original one.

> **Note**: 
> - Compressing files other than ASCII based text files (e.g., audio (.mp3), video (.mp4), pdfs, document (.doc/.docx), etc.) can have little or no effect on the resulting size.
