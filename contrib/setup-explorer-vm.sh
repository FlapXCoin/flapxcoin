#!/usr/bin/env bash
# One-command setup for a FlapX explorer server on Ubuntu 22.04 / 24.04.
#
# Installs and starts:
#   - flapxd, built from source (branch modern-linux-build), run by systemd
#   - MongoDB 8.0
#   - Node.js 24 and the FlapX explorer (nerdlabs-dev/eiquidus-lab), run by pm2
#   - cron jobs that feed new blocks and peers from flapxd into the explorer
#
# Run it as your normal user (not root); it asks for sudo when it needs it:
#
#   bash <(curl -fsSL https://raw.githubusercontent.com/nerdlabs-dev/flapxcoin-dev/modern-linux-build/contrib/setup-explorer-vm.sh)
#
# Safe to run again: it updates and rebuilds flapxd and the explorer, keeps
# your existing flapx.conf, settings.json and passwords, and restarts things.
#
# Optional settings, put in front of the command (e.g. ADDNODES="1.2.3.4" bash <(curl ...)):
#   ADDNODES="ip1 ip2"     extra peers to add to flapx.conf (first run only), e.g. your caught-up node
#   FLAPX_BRANCH=...       flapxcoin-dev branch to build (default: modern-linux-build)
#   EXPLORER_PORT=3001     port the explorer website listens on (default: 3001)
#   SKIP_EXPLORER=1        only set up flapxd
set -euo pipefail

FLAPX_REPO="${FLAPX_REPO:-https://github.com/nerdlabs-dev/flapxcoin-dev}"
FLAPX_BRANCH="${FLAPX_BRANCH:-modern-linux-build}"
EXPLORER_REPO="${EXPLORER_REPO:-https://github.com/nerdlabs-dev/eiquidus-lab}"
EXPLORER_PORT="${EXPLORER_PORT:-3001}"
ADDNODES="${ADDNODES:-}"

SRC_DIR="$HOME/flapxcoin-dev"
DATA_DIR="$HOME/.flapx"
CONF="$DATA_DIR/flapx.conf"
EXPLORER_DIR="$HOME/explorer"
RPC_PORT=60142
P2P_PORT=60042

step() { printf '\n\033[1;36m==> %s\033[0m\n' "$*"; }
note() { printf '    %s\n' "$*"; }
die()  { printf '\n\033[1;31mERROR: %s\033[0m\n' "$*" >&2; exit 1; }

# ---------------------------------------------------------------- checks
[ "$(id -u)" -ne 0 ] || die "Run this as your normal user, not root (it uses sudo when needed)."
command -v sudo >/dev/null || die "sudo is not installed."
. /etc/os-release
case "${ID:-}:${VERSION_CODENAME:-}" in
    ubuntu:noble|ubuntu:jammy) ;;
    *) die "This script supports Ubuntu 22.04 (jammy) and 24.04 (noble); this is ${PRETTY_NAME:-unknown}." ;;
esac
ARCH="$(uname -m)"
if [ -z "${SKIP_EXPLORER:-}" ] && [ "$ARCH" = "x86_64" ] && ! grep -qw avx /proc/cpuinfo; then
    die "This CPU does not report AVX, which MongoDB 5.0+ needs (it would crash with 'Illegal instruction').
       On Proxmox/KVM set the VM's CPU type to 'host' (or x86-64-v3), restart the VM and run this again."
fi
sudo -v   # ask for the password once, up front

MEM_MB=$(awk '/MemTotal/ {print int($2/1024)}' /proc/meminfo)
JOBS=$(( MEM_MB / 1500 )); [ "$JOBS" -ge 1 ] || JOBS=1
[ "$JOBS" -le "$(nproc)" ] || JOBS=$(nproc)

# ---------------------------------------------------------------- packages
step "Installing system packages"
sudo apt-get update -q
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -q \
    build-essential git curl gnupg ca-certificates cron python3 openssl \
    libssl-dev libdb++-dev zlib1g-dev libminiupnpc-dev \
    libboost-system-dev libboost-filesystem-dev libboost-program-options-dev \
    libboost-thread-dev libboost-chrono-dev

# ---------------------------------------------------------------- flapxd
step "Building flapxd from $FLAPX_BRANCH"
if [ -d "$SRC_DIR/.git" ]; then
    git -C "$SRC_DIR" fetch -q --depth 1 origin "$FLAPX_BRANCH"
    git -C "$SRC_DIR" checkout -q -B "$FLAPX_BRANCH" FETCH_HEAD
else
    git clone -q --depth 1 -b "$FLAPX_BRANCH" "$FLAPX_REPO" "$SRC_DIR"
fi
note "commit $(git -C "$SRC_DIR" log -1 --format='%h %s')"
note "compiling with $JOBS job(s), this takes a few minutes..."
make -s -C "$SRC_DIR/src" -f makefile.unix -j"$JOBS" USE_UPNP=0 >/tmp/flapxd-build.log 2>&1 \
    || { tail -30 /tmp/flapxd-build.log; die "flapxd build failed (full log: /tmp/flapxd-build.log)"; }
strip "$SRC_DIR/src/flapxd"
BINARY_CHANGED=1
if [ -f /usr/local/bin/flapxd ] && cmp -s "$SRC_DIR/src/flapxd" /usr/local/bin/flapxd; then BINARY_CHANGED=0; fi
sudo install -m 755 "$SRC_DIR/src/flapxd" /usr/local/bin/flapxd

step "Configuring flapxd ($CONF)"
mkdir -p "$DATA_DIR"
if [ ! -f "$CONF" ]; then
    {
        echo "# Created by setup-explorer-vm.sh"
        echo "server=1"
        echo "listen=1"
        echo "rpcuser=flapxrpc"
        echo "rpcpassword=$(openssl rand -hex 24)"
        echo "rpcport=$RPC_PORT"
        echo "rpcallowip=127.0.0.1"
        echo "logtimestamps=1"
        for n in $ADDNODES; do echo "addnode=$n"; done
    } > "$CONF"
    chmod 600 "$CONF"
    note "created with a new random RPC password"
else
    note "keeping your existing flapx.conf"
    grep -q '^server=1' "$CONF" || echo "server=1" >> "$CONF"
    grep -q '^rpcuser=' "$CONF" || echo "rpcuser=flapxrpc" >> "$CONF"
    grep -q '^rpcpassword=' "$CONF" || echo "rpcpassword=$(openssl rand -hex 24)" >> "$CONF"
fi
RPC_USER=$(sed -n 's/^rpcuser=//p' "$CONF" | tail -1)
RPC_PASS=$(sed -n 's/^rpcpassword=//p' "$CONF" | tail -1)

sudo tee /etc/systemd/system/flapxd.service >/dev/null <<EOF
[Unit]
Description=FlapX daemon
After=network-online.target
Wants=network-online.target

[Service]
User=$USER
ExecStart=/usr/local/bin/flapxd -datadir=$DATA_DIR
ExecStop=/usr/local/bin/flapxd -datadir=$DATA_DIR stop
Restart=on-failure
RestartSec=30
TimeoutStopSec=300
LimitNOFILE=8192

[Install]
WantedBy=multi-user.target
EOF
sudo systemctl daemon-reload
sudo systemctl enable -q flapxd
if systemctl is-active -q flapxd; then
    if [ "$BINARY_CHANGED" = 1 ]; then note "restarting flapxd on the new build"; sudo systemctl restart flapxd; fi
else
    sudo systemctl start flapxd
fi
note "flapxd is $(systemctl is-active flapxd)"

if command -v ufw >/dev/null && sudo ufw status | grep -q "Status: active"; then
    sudo ufw allow "$P2P_PORT/tcp" >/dev/null
    [ -n "${SKIP_EXPLORER:-}" ] || sudo ufw allow "$EXPLORER_PORT/tcp" >/dev/null
    if [ -n "${SKIP_EXPLORER:-}" ]; then note "opened port $P2P_PORT in ufw"; else note "opened ports $P2P_PORT and $EXPLORER_PORT in ufw"; fi
fi

if [ -n "${SKIP_EXPLORER:-}" ]; then
    step "Done (flapxd only)"
    note "Status:  systemctl status flapxd   |   flapxd getinfo   |   tail -f $DATA_DIR/debug.log"
    exit 0
fi

# ---------------------------------------------------------------- MongoDB
step "Installing MongoDB 8.0"
if ! command -v mongod >/dev/null; then
    curl -fsSL https://pgp.mongodb.com/server-8.0.asc | sudo gpg --dearmor --yes -o /usr/share/keyrings/mongodb-server-8.0.gpg
    echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-8.0.gpg ] https://repo.mongodb.org/apt/ubuntu $VERSION_CODENAME/mongodb-org/8.0 multiverse" \
        | sudo tee /etc/apt/sources.list.d/mongodb-org-8.0.list >/dev/null
    sudo apt-get update -q
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -q mongodb-org
else
    note "already installed ($(mongod --version | head -1))"
fi
sudo systemctl enable -q mongod
sudo systemctl start mongod
for i in $(seq 1 30); do
    mongosh --quiet --eval 'db.runCommand({ping:1}).ok' 2>/dev/null | grep -q 1 && break
    [ "$i" -lt 30 ] || die "MongoDB did not start (see: sudo journalctl -u mongod)"
    sleep 2
done

# ---------------------------------------------------------------- Node.js
step "Installing Node.js 24"
NODE_MAJOR=$(node -v 2>/dev/null | sed 's/^v\([0-9]*\).*/\1/' || true)
if [ -z "$NODE_MAJOR" ] || [ "$NODE_MAJOR" -lt 20 ]; then
    curl -fsSL https://deb.nodesource.com/setup_24.x | sudo -E bash - >/dev/null
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -q nodejs
fi
note "node $(node -v), npm $(npm -v)"
command -v pm2 >/dev/null || sudo npm install -g --silent pm2

# ---------------------------------------------------------------- explorer
step "Installing the FlapX explorer ($EXPLORER_DIR)"
if [ -d "$EXPLORER_DIR/.git" ]; then
    git -C "$EXPLORER_DIR" pull -q --ff-only
else
    git clone -q --depth 1 "$EXPLORER_REPO" "$EXPLORER_DIR"
fi
note "commit $(git -C "$EXPLORER_DIR" log -1 --format='%h %s')"
[ -f "$EXPLORER_DIR/settings.json" ] || cp "$EXPLORER_DIR/settings.json.template" "$EXPLORER_DIR/settings.json"

# Fill in the wallet RPC login and the database password; keep a database
# password set by an earlier run, replace the template's public default.
MONGO_PASS=$(python3 - "$EXPLORER_DIR/settings.json" "$RPC_USER" "$RPC_PASS" "$RPC_PORT" "$EXPLORER_PORT" <<'PY'
import re, sys, secrets
path, rpc_user, rpc_pass, rpc_port, web_port = sys.argv[1:]
s = open(path).read()
TEMPLATE_DB_PASS = "Nd^p2d77ceBX!L"

def block(name):
    m = re.search(r'"%s"\s*:\s*\{' % name, s)
    if not m: sys.exit("settings.json has no %s section" % name)
    end = s.index("}", m.end())
    return m.end(), end

def set_key(name, key, value, quote=True):
    global s
    a, b = block(name)
    part = s[a:b]
    val = '"%s"' % value if quote else value
    part, n = re.subn(r'("%s"\s*:\s*)("[^"]*"|\d+)' % key, lambda m: m.group(1) + val, part, count=1)
    if n != 1: sys.exit("settings.json: no %s.%s" % (name, key))
    s = s[:a] + part + s[b:]

def get_key(name, key):
    a, b = block(name)
    m = re.search(r'"%s"\s*:\s*"([^"]*)"' % key, s[a:b])
    return m.group(1) if m else ""

db_pass = get_key("dbsettings", "password")
if not db_pass or db_pass == TEMPLATE_DB_PASS:
    db_pass = secrets.token_hex(24)
set_key("dbsettings", "password", db_pass)
set_key("wallet", "username", rpc_user)
set_key("wallet", "password", rpc_pass)
set_key("wallet", "port", rpc_port, quote=False)
set_key("webserver", "port", web_port, quote=False)
open(path, "w").write(s)
print(db_pass)
PY
)
chmod 600 "$EXPLORER_DIR/settings.json"

note "creating the explorer's database login"
mongosh --quiet explorerdb --eval "
  if (db.getUser('eiquidus')) db.changeUserPassword('eiquidus', '$MONGO_PASS');
  else db.createUser({ user: 'eiquidus', pwd: '$MONGO_PASS', roles: ['readWrite'] });
" >/dev/null

note "installing explorer packages (npm), this takes a minute..."
(cd "$EXPLORER_DIR" && npm install --omit=dev --no-audit --no-fund --loglevel=error >/tmp/explorer-npm.log 2>&1) \
    || { tail -30 /tmp/explorer-npm.log; die "npm install failed (full log: /tmp/explorer-npm.log)"; }

step "Starting the explorer with pm2"
cd "$EXPLORER_DIR"
if pm2 jlist 2>/dev/null | grep -q '"name":"explorer"'; then
    npm run --silent reload-pm2
else
    npm run --silent start-pm2
fi
# start pm2 (and the explorer) at boot
sudo env PATH="$PATH" "$(command -v pm2)" startup systemd -u "$USER" --hp "$HOME" >/dev/null
pm2 save >/dev/null

step "Scheduling explorer sync (cron)"
NODE_BIN=$(command -v node)
CRON_TAG="# flapx-explorer"
( crontab -l 2>/dev/null | grep -v "$CRON_TAG" || true
  echo "*/1 * * * * cd $EXPLORER_DIR && $NODE_BIN scripts/sync.js update > /dev/null 2>&1 $CRON_TAG"
  echo "*/5 * * * * cd $EXPLORER_DIR && $NODE_BIN scripts/sync.js peers > /dev/null 2>&1 $CRON_TAG"
) | crontab -
note "new blocks every minute, peers every 5 minutes"

# ---------------------------------------------------------------- summary
IP=$(hostname -I 2>/dev/null | awk '{print $1}')
step "All set"
cat <<EOF
    Explorer:  http://${IP:-this-server}:$EXPLORER_PORT
               (it fills in as flapxd downloads the chain; blocks show up as they arrive)

    flapxd:    systemctl status flapxd      flapxd getblockcount      tail -f $DATA_DIR/debug.log
    Explorer:  pm2 status                   pm2 logs explorer
    MongoDB:   systemctl status mongod

    Config:    $CONF
               $EXPLORER_DIR/settings.json   (RPC and database passwords are filled in)

    Peers connect to this node on TCP $P2P_PORT; forward it on your router to make this a public node.
    To update everything later, run the same command again.
EOF
