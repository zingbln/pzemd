Install: 

sudo apt-get update
sudo apt-get install libcurl4-openssl-dev libmodbus-dev

git clone https://github.com/zingbln/pzemd

cd pzemd
make
edit pzemd.cfg
cp pzemd.service /etc/systemd/system
sudo systemctl start pzemd
sudo systemctl enable pzemd
