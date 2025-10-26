# tmux-snippets
Plugin for tmux to store frequently used terminal commands.

# Installation

1. Download latest release
	```bash
	wget https://github.com/matveigavrilov1/tmux-snippets/releases/download/v0.1.0/tmux-snippets-0.1.0-Release-a7bab3d.tar.gz
	```

2. Unzip archive to tmux plugins dirextory
	```
	mkdir -p ~/.tmux/plugins/tmux-snippets && tar -xzf tmux-snippets-0.1.0-Release-a7bab3d.tar.gz -C ~/.tmux/plugins/tmux-snippets --strip-components=1
	```

3. Edit `.tmux.conf`
	- With tpm
		```
		set -g @plugin 'tmux-plugins/tmux-snippets'
		```

	- Without tpm
		```
		run '~/.tmux/plugins/tmux-snippets/snippets.tmux'
		```

4. Reload tmux or use folowing command to activate plugin
	```
	tmux source ~/.tmux.conf
	```

# Usage

To call plugin `C-b T` used by default