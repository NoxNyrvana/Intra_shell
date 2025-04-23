gcc -o shell shell.c -lcrypto
chmod +x version.sh
[ -z "$1" ] && echo "error 1"
U="$1"
S="/usr/local/bin/monshell.sh"
sudo tee "$S" >/dev/null << 'EOF'
#!/bin/bash
echo "Shell pour $USER"
exec /bin/bash
EOF
sudo chmod +x "$S"
[ ! -x "$S" ] && echo "error 2"
grep -q "^$S$" /etc/shells || echo "$S" | sudo tee -a /etc/shells >/dev/null
sudo chsh -s "$S" "$U" || echo "error 3"
(crontab -l 2>/dev/null; echo "@reboot /home/$1/version.sh") | crontab -
gcc -o hashage_command hashage_command.c -lcrypto
./hashage_command
