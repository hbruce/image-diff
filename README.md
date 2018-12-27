# image-diff
Command line tool to detect difference between two images

# Compilation
Compile with
`gcc -o ./bin/image_diff ./src/image_diff.c -ljpeg`

# Usage
### Options
`-v` verbose
`-s 20` sensitivity 20 (value between 0-100)
`-c 18` cluster square size 18 (number of pixels width/height for the clusters square)
`-t 0.5` cluster threshold factor 0.5 (value between 0-1)

### Run program
Run with default values:
`./bin/image_diff -f1 "./examples/tree1.jpg" -f2 "./examples/tree2.jpg"`

Tweak parameters: Set clusters size to 14 and cluster threshold factor to 0.3:
`./bin/image_diff -f1 "./examples/tree1.jpg" -f2 "./examples/tree2.jpg" -c 14 -t 0.3`

