#include "ActionPilot.h"
#include "ActionWithArguments.h"
#include "ActionRegister.h"
#include "PlumedCommunicator.h"
#include <cassert>

using namespace std;

namespace PLMD{

//+PLUMEDOC GENERIC DUMPDERIVATIVES
/**

Dump the derivatives of one or more objects with respect to their input parameters
on a file. For collective variables, these are the derivatives of the collective
variable (or of one of its components) with respect to atom positions and to cell
vectors (virial-like form). For functions, there are the the derivatives with respect
the function arguments.

\par Syntax
\verbatim
DUMPDERIVATIVES ARG=what [STRIDE=s] [FILE=file]
\endverbatim
It can print at the same time derivatives of more than one object, but they should
have the same number of parameters. Typically, this can be used to test numerical
derivatives againts analytical ones

\par Example
The following input is writing on file deriv distanceB both
the analytical and numerical derivatives of distance between atoms 1 and 2.
\verbatim
DISTANCE ATOM=1,2 LABEL=distance
DISTANCE ATOM=1,2 LABEL=distanceN NUMERICAL_DERIVATIVES
DUMPDERIVATIVES ARG=distance,distanceN STRIDE=1 FILE=deriv
\endverbatim

(See also \ref DISTANCE)

*/
//+ENDPLUMEDOC

class GenericDumpDerivatives :
public ActionPilot,
public ActionWithArguments
{
  string file;
  string fmt;
  FILE* fp;
public:
  void calculate(){};
  GenericDumpDerivatives(const ActionOptions&);
  void apply(){};
  void update();
  ~GenericDumpDerivatives();
};

PLUMED_REGISTER_ACTION(GenericDumpDerivatives,"DUMPDERIVATIVES")

void GenericDumpDerivatives::registerKeywords(Keywords& keys){
  Action::registerKeywords(keys);
  ActionPilot::registerKeywords(keys);
  ActionWithArguments::registerKeywords(keys);
  keys.add("compulsory","STRIDE","1","the frequency with which the derivatives should be output");
  keys.add("compulsory","FILE","the name of the file on which to output the derivatives");
  keys.add("compulsory","FMT","%15.10f","the format with which the derivatives should be output");
}

GenericDumpDerivatives::GenericDumpDerivatives(const ActionOptions&ao):
Action(ao),
ActionPilot(ao),
ActionWithArguments(ao),
fmt("%15.10f"),
fp(NULL)
{
  parse("FILE",file);
  assert(file.length()>0);
  parse("FMT",fmt);
  fmt=" "+fmt;
  if(comm.Get_rank()==0){
    fp=fopen(file.c_str(),"wa");
    log.printf("  on file %s\n",file.c_str());
    log.printf("  with format %s\n",fmt.c_str());
    fprintf(fp,"%s","#! FIELDS time parameter");
    const std::vector<Value*>& arguments(getArguments());
    assert(arguments.size()>0);
    unsigned npar=arguments[0]->getDerivatives().size();
    assert(npar>0);
    for(unsigned i=1;i<arguments.size();i++){
      assert(npar==arguments[i]->getDerivatives().size());
    }
    for(unsigned i=0;i<arguments.size();i++){
      fprintf(fp," %s",arguments[i]->getFullName().c_str());
    };
    fprintf(fp,"%s","\n");
  }
  checkRead();
}


void GenericDumpDerivatives::update(){
  if(comm.Get_rank()!=0)return;
  const std::vector<Value*>& arguments(getArguments());
  unsigned npar=arguments[0]->getDerivatives().size();
  for(unsigned ipar=0;ipar<npar;ipar++){
    fprintf(fp," %f",getTime());
    fprintf(fp," %u",ipar);
    for(unsigned i=0;i<getNumberOfArguments();i++){
      fprintf(fp,fmt.c_str(),arguments[i]->getDerivatives()[ipar]);
    };
    fprintf(fp,"\n");
  }
}

GenericDumpDerivatives::~GenericDumpDerivatives(){
  if(fp) fclose(fp);
}

}


