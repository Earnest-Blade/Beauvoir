# TODO

## shader.c
- Improve user's shader usage
    - more friendly shader stages
    - more friendly uniform and uniform block registering
    - create a more easy way to connect shaders to textures

## buffer.c
- Improve string memory safety
    - add max number to string concat
    - add growth factor based strings
    - add max capacity for strings

## image.c
- Improve image compatibility
    - add more PSD cases
    - add more BMP compressions
    - create custom PNG loader ?

- Bugs
    - Flipping PSD image does not work
    - Loading too much images create a sort of mem overflow