make
clear

echo "./av3 ####################################################"
./av3
echo "./reader ####################################################"
./reader
echo "./reader s ####################################################"
./reader s

echo "time ./av3 s0 # pila inexistente #################################"
time ./av3 s0
echo "./reader s0 ####################################################"
./reader s0

echo "time ./av3 s0 # 2ª ejecución misma pila ##########################"
time ./av3 s0
echo "./reader s0 ####################################################"
./reader s0

echo "./av3 s1-14el0 ################################################"
# pila con 14 elementos
./av3 s1-14el0
echo "./reader s1-14el0 ###############################################"
# los calculos han de hacerse sobre los 10 superiores
./reader s1-14el0

echo "./av3 s2-6el0 ####################################################"
#pila con 6 elementos
./av3 s2-6el0
echo "./reader s2-6el0 ####################################################"
./reader s2-6el0
echo "./av3 s2-6el0 ####################################################"
./av3 s2-6el0
echo "./reader s2-6el0 ####################################################"
echo "los cálculos han de añadir 10.000.000 a la suma anterior"
./reader s2-6el0
