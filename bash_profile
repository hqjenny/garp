if [ -e /share/b/bin/instructional.bashrc ]
  then
       source /share/b/bin/instructional.bashrc
fi

umask 077
source ~cs250/tools/cs250.bashrc.select

export SBT_OPTS="-Xmx4G -Xms4G"
export JAVA_OPTS="-Xmx4G -Xms4G"

