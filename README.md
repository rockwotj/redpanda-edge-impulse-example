# Edge Impulse + Redpanda Connect Example

This is a small example of Edge Impulse + Redpanda Connect working together for an easy to use Edge AI streaming pipeline.

This is the output of myself going through the [sound classification tutorial](https://docs.edgeimpulse.com/docs/tutorials/end-to-end-tutorials/responding-to-your-voice) and outputting the resulting project as a C++ library. The C++ library is unpacked into the `my_edge_impulse_project/` directory. I have then built a small integration point using Redpanda's C++ SDK (found [here](https://github.com/redpanda-data/redpanda/tree/dev/src/transform-sdk/cpp)) using the edge impulse library. You can see the code and build system at `src/transform.cc` and `CMakeLists.txt` respectively.

The Data Transform takes in a JSON array representing the raw feature data for the classifier, and outputs the score for each label.

## Building Code for WASI/Wasm

The following command will compile a wasm32-wasi artifact at `build/app`. The artifact can be deployed to Redpanda or Redpanda Connect.

```
docker run -v `pwd`:/src -w /src ghcr.io/webassembly/wasi-sdk /bin/bash -c 'cmake -Bbuild && cmake --build build'
```

## Deploying to Redpanda Connect

The simplest way to run this is using the Redpanda Connect configuration file that can be found at `connect.yaml` (make sure you've built the SDK first!).

The follow command sends some example data (redpanda labelled and noise labelled respectively) into connect to be classified and reported on stdout.

```
cat example_data.txt | docker run --rm -i -v $(pwd)/connect.yaml:/connect.yaml -v $(pwd)/build/app:/app.wasm docker.redpanda.com/redpandadata/connect run
```
