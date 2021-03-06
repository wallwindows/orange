/*
#include "gsl/gsl_matrix.h"
#include "gsl/gsl_vector.h"
*/
#include <vector>

#include "orange.hpp"
#include "vars.hpp"
#include "examplegen.hpp"

/*
#ifndef _DEBUG
// I prefer to see exceptions when debugging
extern "C" void my_gsl_error_handler(const char *reason, const char *file, int line, int)
{ raiseErrorWho("GSL", "%s (%s:%i)", reason, file, line); }

gsl_error_handler_t *fooerrh = gsl_set_error_handler(my_gsl_error_handler);
#endif
*/

void parseMatrixContents(PExampleGenerator egen, const int &weightID, const char *contents, const int &multiTreatment,
                         bool &hasClass, bool &classVector, bool &multiclassVector, bool &weightVector, bool &classIsDiscrete, int &columns,
                         vector<bool> &include)
{
  hasClass = bool(egen->domain->classVar);

  columns = 0;
  int classIncluded = 0, attrsIncluded = 0, weightIncluded = 0, multiclassIncluded = 0;
  bool attrsRequested = false, classRequested = false, weightRequested = false;
  bool multiclassRequested = false;
  const char *cp;
  for(cp = contents; *cp && (*cp!='/'); cp++) {
    switch (*cp) {
      case 'A': attrsRequested = true;
      case 'a': attrsIncluded++;
                break;

      case 'C': classRequested = true;
      case 'c': classIncluded++;
                break;

      case 'W': weightRequested = true;
      case 'w': weightIncluded++;
                break;

      case 'M': multiclassRequested = true;
      case 'm': multiclassIncluded++;
                break;

      case '0':
      case '1': columns++;
                break;
      default:
        raiseErrorWho("parseMatrixContents", "unrecognized character '%c' in format string '%s')", *cp, contents);
    }
  }

  classVector = false;
  multiclassVector = false;
  weightVector = false;

  if (*cp)
    while(*++cp)
      switch (*cp) {
        case 'A':
        case 'a': raiseErrorWho("parseMatrixContents", "invalid format string (attributes on the right side)");

        case '0':
        case '1': raiseErrorWho("parseMatrixContents", "invalid format string (constants on the right side)");

        case 'c': classVector = hasClass; break;
        case 'C': classVector = true; break;

        case 'w': weightVector = (weightID != 0); break;
        case 'W': weightVector = true; break;

        case 'm': multiclassVector = (egen->domain->classVars->size() != 0); break;
        case 'M': multiclassVector = true; break;
        default:
          raiseErrorWho("parseMatrixContents", "unrecognized character '%c' in format string '%s')", *cp, contents);
      }


  if (classIncluded || classVector) {
    if (hasClass) {
      TEnumVariable *eclassVar = egen->domain->classVar.AS(TEnumVariable);
      classIsDiscrete = eclassVar != NULL;
      if (classIsDiscrete) {
        if ((eclassVar->values->size()>2) && (multiTreatment != 1))
          raiseErrorWho("parseMatrixContents", "multinomial classes are allowed only when explicitly treated as ordinal");  
      }
      else {
        if (egen->domain->classVar->varType != TValue::FLOATVAR)
          raiseErrorWho("parseMatrixContents", "unsupported class type");
      }  

      columns += classIncluded;
    }
    else if (classRequested || classVector)
      raiseErrorWho("parseMatrixContents", "classless domain");
  }


  if (weightIncluded || weightVector) {
    if (weightID)
      columns += weightIncluded;
  }

  if (multiclassIncluded || multiclassVector) {
      columns += multiclassIncluded * egen->domain->classVars->size();
  }

  include.clear();

  if (attrsIncluded) {
    int attrs_in = 0;

    const_PITERATE(TVarList, vi, egen->domain->attributes) {
      if ((*vi)->varType == TValue::FLOATVAR) {
        attrs_in++;
        include.push_back(true);
      }
      else if ((*vi)->varType == TValue::INTVAR) {
        if ((*vi).AS(TEnumVariable)->values->size() == 2) {
          attrs_in++;
          include.push_back(true);
        }
        else
          switch (multiTreatment) {
            case 0:
              include.push_back(false);
              break;

            case 1:
              attrs_in++;
              include.push_back(true);
              break;

            default:
              raiseErrorWho("parseMatrixContents", "attribute '%s' is multinomial", (*vi)->get_name().c_str());
          }
      }
      else {
        attrs_in++;
        include.push_back(true);
        raiseWarning(PyExc_OrangeKernelWarning, "attribute '%s' is of unsupported type", (*vi)->get_name().c_str());
      }
    }

    if (attrsRequested && !attrs_in)
      raiseErrorWho("parseMatrixContents", "the domain has no (useful) attributes");

    columns += attrs_in * attrsIncluded;
  }
}

