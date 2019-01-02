# image-diff
Command line tool to detect difference between two images. Takes two jpg images as argument and creates a new image with the difference marked.

# Compilation
Compile with

`gcc -o ./bin/image_diff ./src/image_diff.c -ljpeg`

# Usage
### Arguments
`-f1 "file1.jpg"` input file 1

`-f2 "file2.jpg"` input file 2

### Optional arguments
`-o "diff.jpg"` output file (defaults to "file1_diff.jpg" if not set)

`-m "mask.jpg"` mask file. Code will only check for difference where mask image pixel is 100% white (#FFFFFF)

`-v` verbose

`-s 20` sensitivity 20 (value between 0-100)

`-c 18` cluster square size 18 (number of pixels width/height for the clusters square)

`-t 0.5` cluster threshold factor 0.5 (value between 0-1)

`-g` convert color image to grayscale (if one of the images are grayscale)

`-h` return cluster hits coordinates (upper left corner of each cluster)

### Run program
Run with default values:

`./bin/image_diff -f1 "./examples/tree1.jpg" -f2 "./examples/tree2.jpg"`

Tweak parameters! Set clusters size to 14 and cluster threshold factor to 0.3:

`./bin/image_diff -f1 "./examples/tree1.jpg" -f2 "./examples/tree2.jpg" -c 14 -t 0.3`

### Output

If any clusters with difference was detected, a new jpeg file will be created based on file 2, with clusters marked. If no output filename was passed as argument, the new file will be named the same as file 2 but ending with "_diff.jpg".