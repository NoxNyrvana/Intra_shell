gcc -o shell shell.c -lcrypto
chmod +x version.sh
(crontab -l ; echo "@reboot /home/$USER/version.sh") | crontab -
crontab -l
gcc -o hashage_command hashage_command.c -lcrypto
./hashage_command
[ -z "$1" ] && exit 1
U="$1"
S="/usr/local/bin/monshell.sh"
[ ! -x "$S" ] && exit 2
grep -q "^$S$" /etc/shells || echo "$S" | sudo tee -a /etc/shells >/dev/null
sudo chsh -s "$S" "$U" || exit 3
exit 0
