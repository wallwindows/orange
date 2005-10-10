cd /home/orange
cvs update
chmod +x ./install-scripts/linux/downloadinstallregress.sh ./install-scripts/linux/testOrange.sh

# prepare sources
cd /home/orange/daily
rm -Rf orange 
cvs -d :pserver:cvso@estelle.fri.uni-lj.si:/cvs checkout -d orange orange
cvs -d :pserver:cvso@estelle.fri.uni-lj.si:/cvs checkout -d orange/source source
cp /home/orange/install-scripts/linux/setup.py /home/orange/daily/orange

# build
cd /home/orange/daily/orange
VER='1.test'
cat setup.py | sed s/"OrangeVer=\"ADDVERSION\""/"OrangeVer=\"Orange-$VER\""/ > new.py
mv -f new.py setup.py

echo `date` > ../output.log
if ! python setup.py compile >> ../output.log 2>&1 ; then
  cat compiling.log >> ../output.log
  mail -s "Linux: ERROR compiling Orange" tomaz.curk@fri.uni-lj.si < ../output.log
  cat ../output.log
  echo -e "\n\nERROR compiling, see log above"
  exit 1
fi

# install
cd /home/orange/daily/orange
python setup.py install --orangepath=/home/orange/daily/test_install &> install.log

# regression test
cd /home/orange/daily/orange
if ! /home/orange/install-scripts/linux/testOrange.sh >> ../regress.log 2>&1; then
  cd /home/orange/daily
  mail -s "Linux: ERROR regression tests (compile OK) Orange" tomaz.curk@fri.uni-lj.si < regress.log
  cat regress.log
  echo regression tests failed
else
  cd /home/orange/daily
  cat output.log >> install.log
  mv install.log ../output.log
  mail -s "Linux: Orange compiled successfully" tomaz.curk@fri.uni-lj.si < ../output.log
  cat output.log
  echo compiling was successful
fi

