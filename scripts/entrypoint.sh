#!/usr/bin/env bash

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CURRENT_PANE=$(tmux display-message -p '#{pane_id}')
# echo $CURRENT_PANE
tmux new-window -n "snippets" "
	${CURRENT_DIR}/../tmux-snippets-ui ${CURRENT_PANE}
	tmux kill-window
"

#  tmux send-keys -t "$CURRENT_PANE" "ls -la" Enter