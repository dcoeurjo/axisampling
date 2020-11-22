Image stippling to SVG
===

This code creates stippling effects using [low discrepancy blue noise](https://github.com/dcoeurjo/LowDiscBlueNoise) sampling. The output is a valid SVG file you
can plot using an axis plotter.

Have a look the command line options for more details.

The original idea comes from [Paul Rickards](https://shop.paulrickards.com) artwork.


Building
--------

The code is a standalone c++ code:

```
mkdir build
cd build
cmake ..
make
```

Examples
-------

![](examples/2.png)
![](examples/1.png)
