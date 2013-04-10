hostlist="minervagpvm02 novagpvm02 novagpvm03 fermicloud027 bel-kwinith "

cat <<'EOF'
Build bits:
-----------

mkdir /tmp/$USER$$
cd /tmp/$USER$$
setup git
git clone ssh://p-ifdhc@cdcvs.fnal.gov/cvs/projects/ifdhc/ifdhc.git
cd ifdhc
. ups/build_node_setup.sh
make all install
setup -. ifdhc
python <<XXXX
import ifdh
XXXX
upd addproduct -r `pwd` -M ups -m ifdhc.table $DECLAREBITS ifdhc version
EOF

multixterm -xc "ssh %n" $hostlist
