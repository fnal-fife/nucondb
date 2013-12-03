hostlist="minervagpvm02 novagpvm02 novagpvm03 fermicloud050 fermicloud065 "

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
setup upd
VERSION=v1_2_9
make distrib
#    
upd addproduct -T ifdhc.tar.gz  -M ups -m ifdhc.table $DECLAREBITS ifdhc $VERSION
upd addproduct -T ifbeam.tar.gz -M ups -m ifbeam.table $DECLAREBITS ifbeam $VERSION
cd 
rm -rf /tmp/$USER$$
EOF

multixterm -xc "ssh %n" $hostlist
