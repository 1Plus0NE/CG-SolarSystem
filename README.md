# Solar System - Computer Graphics

In order to execute the following project you may run the next commands.

Build the engine in the `engine/build` directory:

```bash
cmake ..
make
```

Build the generator in the `generator/build` directory:

```bash
cmake ..
make
```

To generate test models, run:

```bash
./generator <shape> <parameters> <output_file>
```

To view the generated environment, run in the `engine/build`:

```bash
./engine test_1_4.xml
```