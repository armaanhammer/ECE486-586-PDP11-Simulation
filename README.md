# ECE486-586-PDP11-Simulation

navigate to `/src/` and run `make` which generates the PDPSim executable

in `/src/` run:
```
./PDPSim (file path) <-R for register printing> <-M for memory printing>
```

For example, to test `MOV.ascii` with all flags, run:
```
./PDPSim ../assembly\ .mac\ files/MOV.ascii -M -R
```
