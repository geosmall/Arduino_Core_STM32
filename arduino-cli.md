
## Board Info

```console
arduino-cli.exe board details --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE,opt=o3std
```

## Compile (Debug, No Optimization, Verbose, ./build)

```console
arduino-cli.exe compile --fqbn STMicroelectronics:stm32:Nucleo_64:opt=o0std,dbg=enable_sym,pnum=NUCLEO_F411RE -v  --build-path .\build
```