# xiaomi-1080-360-motor-control

Provides a way controlling the motor directly on the [MJSXJ02CM camera](https://www.mi.com/global/camera-360).

## Getting started

1. Is it the same as mine?

[![MJSXJ02CM camera](https://i.imgur.com/3fOE6ZR.png)](https://www.mi.com/global/camera-360)

2. Get a shell

Install these mods to get a shell from within your camera:  
https://github.com/telmomarques/xiaomi-360-1080p-hacks

3. Install the toolchain

```shell
 sudo apt-get install gcc-arm-linux-gnueabihf libc6-armhf-cross libc6-dev-armhf-cross binutils-arm-linux-gnueabi
```

4. Bring your own libs (optional)

If you want to use your own libs, you can get it from the camera using [`pull_libs`](./pull_libs). 


5. Clone the repo
```git clone
 git clone https://github.com/CSchlipp/xiaomi-1080-360-motor-control.git
```


6. Kitchen is ready

```shell
make
```

## Usage
### Calibrate
Calibrate Cam and move to center:  
```./motor calibrate 0```

### Move
Move Cam up by 10 steps:  
```./motor up 10```  
Move Cam down by 10 steps:  
```./motor down 10```  
Move Cam left by 10 steps:  
```./motor left 10```  
Move Cam right by 10 steps:  
```./motor right 10```

### Store Present
Store the current position into present 1:  
```./motor store 1```  
Available presents: {1 ... 8}

### Go to stored position
Move Cam to stored position 1:  
```./motor goto 1```  
Available presents: {1 ... 8}