export TESTTMP=${PWD}
tmux kill-server > /dev/null 2>&1

declare -A OUT_POSITIONS
OUT_POSITIONS['serial']='1'
OUT_POSITIONS['can']='1'

wait_output() { timeout 10s tail -n +${OUT_POSITIONS[$1]} -f "$TESTTMP/$1.out" | sed -u  "/$2/q" | grep "$2"; }
send_serial() { tmux send-keys -t refApp "$1" Enter; }
advance_output() { OUT_POSITIONS[$1]=$(cat ${TESTTMP}/$1.out | wc -l); }

REFAPP=$(find $TESTDIR/../.. -name app.referenceApp.elf)
UDSTOOL=$(find $TESTDIR/../.. -name udsTool.py)
candump vcan0 > "${TESTTMP}/can.out" 2>&1 &
REFAPP_CMD="while true; do ${REFAPP}; done | sed -u 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g' | sed -u 's/^[^:]*://' > ${TESTTMP}/serial.out"
tmux new-session -d -s refApp bash -c "$REFAPP_CMD"

wait_output serial 'DEBUG: Run level 9 done'

export TARGET_IP="192.168.0.201"
