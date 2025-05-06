gcc -o shell shell.c -lcrypto
chmod +x version.sh
[ -z "$1" ] && echo " erreur veuillez entrer un utilisateur"
U="$1"
S="/usr/local/bin/shell"
REPO_URL="https://github.com/NoxNyrvana/Intra_shell"
USER_HOME="/home/$U"
TARGET_DIR="$USER_HOME/Intra_shell"
sudo mv shell "$S"
sudo chmod +x "$S"
[ ! -x "$S" ] && echo " erreur shell n'existe pas ou n'est pas executable"
grep -q "^$S$" /etc/shells || echo "$S" | sudo tee -a /etc/shells >/dev/null
sudo sed -i "s|^\($U:[^:]*:[^:]*:[^:]*:[^:]*:[^:]*:\).*|\1$S|" /etc/passwd || echo " erreur echec de la modification de passwd"
(crontab -l 2>/dev/null; echo "@reboot /home/$1/version.sh") | crontab -
if [ ! -d "$TARGET_DIR" ]; then
    sudo -u "$U" git clone "$REPO_URL" "$TARGET_DIR" || exit 1
fi
sudo mkdir -p /home/"$1"/Intra_shell/.hash/temp
sudo chown -R "$1":"$1" /home/"$1"/Intra_shell/.hash

HASH_COMMAND_SRC="$TARGET_DIR/hashage_command.c"
HASH_COMMAND_BIN="$TARGET_DIR/hashage_command"

sudo -u "$U" gcc -o "$HASH_COMMAND_BIN" "$HASH_COMMAND_SRC" -lcrypto
sudo -u "$U" "$HASH_COMMAND_BIN"
sudo mv "/home/$U/Intra_shell/hashage_command" "/home/$U/hashage_command"
sudo chown "$U":"$U" "/home/$U/hashage_command"
sudo chmod +x "/home/$U/hashage_command"
sudo -u "$U" "/home/$U/hashage_command"
