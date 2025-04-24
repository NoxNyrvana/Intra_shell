gcc -o shell shell.c -lcrypto
chmod +x version.sh
[ -z "$1" ] && echo " erreur veuillez entrer un utilisateur"
U="$1"
S="/usr/local/bin/shell"
sudo mv shell "$S"
sudo chmod +x "$S"
[ ! -x "$S" ] && echo " erreur shell n'existe pas ou n'est pas executable"
grep -q "^$S$" /etc/shells || echo "$S" | sudo tee -a /etc/shells >/dev/null
sudo sed -i "s|^\($u:[^:]*:[^:]*:[^:]*:[^:]*:[^:]*:\).*|\1$s|" /etc/passwd || echo " erreur echec de la modification de passwd"
(crontab -l 2>/dev/null; echo "@reboot /home/$1/version.sh") | crontab -
gcc -o hashage_command hashage_command.c -lcrypto
./hashage_command
