sudo mknod -m 666 /dev/vmotor_dev c 508 0
sudo mknod -m 666 /dev/ir_dev c 509 0
sudo mknod -m 666 /dev/gas_dev c 511 0
sudo mknod -m 666 /dev/buzzer_dev c 202 0
sudo mknod -m 666 /dev/motor_dev c 500 0
sudo mknod -m 666 /dev/humidity_dev c 501 0
cd vmotor_dev
sudo rmmod vmotor_dev
make clean
make
sudo insmod vmotor_dev.ko
cd ../ir_dev
sudo rmmod ir_dev
make clean
make
sudo insmod ir_dev.ko
cd ../gas_dev
sudo rmmod gas_dev
make clean
make
sudo insmod gas_dev.ko
cd ../buzzer_dev
sudo rmmod buzzer_dev
make clean
make
sudo insmod buzzer_dev.ko
cd ../humidity_dev
sudo rmmod humidity_dev
make clean
make
sudo insmod humidity_dev.ko
cd ../motor_dev
sudo rmmod motor_dev
make clean
make
sudo insmod motor_dev.ko
cd ../app
rm app1
rm app2
rm app3 
gcc -o app1 app1.c -lm
gcc -o app2 app2.c
gcc -o app3 app3.c
