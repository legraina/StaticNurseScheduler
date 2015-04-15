/*
 * ScipModeler.h:
 *    Tools to create a SCIP problem.
 *    In general, when you want to create a new scip object,
 *    you have to give a pointer to the pointer of the object.
 *
 *  Created on: 2015-02-23
 *      Author: legraina
 */

#ifndef SRC_MODELER_H_
#define SRC_MODELER_H_

/* standard library includes */
#include <cfloat>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <typeinfo>

#include "MyTools.h"

/* namespace usage */
using namespace std;

/*
 * My Modeling objects
 * If the object is added to the vector objects_ of the Modeler, the modeler will also delete it at the end.
 */
struct MyObject {
   MyObject(const char* name):id_(s_count) {
      ++s_count;
      char* name2 = new char[255];
      strncpy(name2, name, 255);
      name_ = name2;
   }
   MyObject(const MyObject& myObject):id_(myObject.id_) {
      char* name2 = new char[255];
      strncpy(name2, myObject.name_, 255);
      name_ = name2;
   }
   virtual ~MyObject(){ delete[] name_; }
   //count object
   static unsigned int s_count;
   //for the map rotations_
   int operator < (const MyObject& m) const { return this->id_ < m.id_; }

   const char* name_;
private:
   const unsigned int id_;
};

/*
 * My pricer
 */
struct MyPricer{
   MyPricer(const char* name): name_(name){ }
   virtual ~MyPricer() { }

   //name of the pricer handler
   //
   const char* name_;

   /* perform pricing */
   //return true if optimal
   virtual bool pricing(double bound=0)=0;
};
/*
 * My branching rule
 */
struct MyBranchingRule{
   MyBranchingRule(const char* name): name_(name){ }
   virtual ~MyBranchingRule() { }

   //name of the branching rule handler
   //
   const char* name_;

   /* compute logical fixing decisions */
   virtual void logical_fixing(vector<MyObject*>& fixingCandidates)=0; // stores the next candidates for logical fixing

   /* compute branching decisions */
   virtual void branching_candidates(vector<MyObject*>& branchingCandidates)=0; // stores the next candidates for branching
};
/*
 * Var types
 */
enum VarType {VARTYPE_CONTINUOUS, VARTYPE_INTEGER, VARTYPE_BINARY};

class Modeler {
public:

   Modeler(){ }

   virtual ~Modeler(){
      for(MyObject* object: objects_)
         delete object;
   }

   //solve the model
   virtual int solve(bool relaxation = false)=0;

   //Add a pricer
   virtual int addObjPricer(MyPricer* pPricer)=0;

   //Add a branching rule
   virtual int addBranchingRule(MyBranchingRule* pBranchingRule)=0;

   /*
    * Create variable:
    *    var is a pointer to the pointer of the variable
    *    var_name is the name of the variable
    *    lhs, rhs are the lower and upper bound of the variable
    *    vartype is the type of the variable: VARTYPE_CONTINUOUS, VARTYPE_INTEGER, VARTYPE_BINARY
    */

   virtual int createVar(MyObject** var, const char* var_name, double objCoeff,
      double lb, double ub, VarType vartype, double score)=0;

   inline void createPositiveVar(MyObject** var, const char* var_name, double objCoeff, double score = 0, double ub = DBL_MAX){
      createVar(var, var_name, objCoeff, 0.0, ub, VARTYPE_CONTINUOUS, score);
   }

   inline void createIntVar(MyObject** var, const char* var_name, double objCoeff, double score = 0, double ub = DBL_MAX){
      createVar(var, var_name, objCoeff, 0, ub, VARTYPE_INTEGER, score);
   }

   inline void createBinaryVar(MyObject** var, const char* var_name, double objCoeff, double score = 0){
      createVar(var, var_name, objCoeff, 0.0, 1.0, VARTYPE_BINARY, score);
   }

   virtual int createColumnVar(MyObject** var, const char* var_name, double objCoeff, double dualObj,
      double lb, double ub, VarType vartype, double score)=0;

   inline void createPositiveColumnVar(MyObject** var, const char* var_name, double objCoeff, double dualObj = 99999, double score = 0, double ub = DBL_MAX){
      createColumnVar(var, var_name, objCoeff, dualObj, 0.0, ub, VARTYPE_CONTINUOUS, score);
   }

   inline void createIntColumnVar(MyObject** var, const char* var_name, double objCoeff, double dualObj = 99999, double score = 0, double ub = DBL_MAX){
      createColumnVar(var, var_name, objCoeff, dualObj, 0, ub, VARTYPE_INTEGER, score);
   }

   inline void createBinaryColumnVar(MyObject** var, const char* var_name, double objCoeff, double dualObj = 99999, double score = 0){
      createColumnVar(var, var_name, objCoeff, dualObj, 0.0, 1.0, VARTYPE_BINARY, score);
   }

   /*
    * Create linear constraint:
    *    con is a pointer to the pointer of the constraint
    *    con_name is the name of the constraint
    *    lhs, rhs are the lower and upper bound of the constraint
    *    nonZeroVars is the number of non-zero coefficients to add to the constraint
    *    vars is an array of pointers to the variables to add to the constraints (with non-zero coefficient)
    *    coeffs is the array of coefficient to add to the constraints
    */

   virtual int createConsLinear(MyObject** cons, const char* con_name, double lhs, double rhs,
      vector<MyObject*> vars = {}, vector<double> coeffs = {})=0;

   //Add a lower or equal constraint
   inline void createLEConsLinear(MyObject** cons, const char* con_name, double rhs,
      vector<MyObject*> vars = {}, vector<double> coeffs = {}){
      createConsLinear(cons, con_name, DBL_MIN, rhs, vars, coeffs);
   }

   //Add a greater or equal constraint
   inline void createGEConsLinear(MyObject** cons, const char* con_name, double lhs,
      vector<MyObject*> vars = {}, vector<double> coeffs = {}){
      createConsLinear(cons, con_name, lhs, DBL_MAX, vars, coeffs);
   }

   //Add an equality constraint
   inline void createEQConsLinear(MyObject** cons, const char* con_name, double eq,
      vector<MyObject*> vars = {}, vector<double> coeffs = {}){
      createConsLinear(cons, con_name, eq, eq, vars, coeffs);
   }

   //Add final linear constraints
   virtual int createFinalConsLinear(MyObject** cons, const char* con_name, double lhs, double rhs,
      vector<MyObject*> vars = {}, vector<double> coeffs = {})=0;

   inline void createFinalLEConsLinear(MyObject** cons, const char* con_name, double rhs,
      vector<MyObject*> vars = {}, vector<double> coeffs = {}){
      createFinalConsLinear(cons, con_name, DBL_MIN, rhs, vars, coeffs);
   }

   inline void createFinalGEConsLinear(MyObject** cons, const char* con_name, double lhs,
      vector<MyObject*> vars = {}, vector<double> coeffs = {}){
      createFinalConsLinear(cons, con_name, lhs, DBL_MAX, vars, coeffs);
   }

   inline void createFinalEQConsLinear(MyObject** cons, const char* con_name, double eq,
      vector<MyObject*> vars = {}, vector<double> coeffs = {}){
      createFinalConsLinear(cons, con_name, eq, eq, vars, coeffs);
   }

   /*
    * Add variables to constraints
    */

   virtual int addCoefLinear(MyObject* cons, MyObject* var, double coeff, bool transformed=false)=0;

   /*
    * Add new Column to the problem
    */

   inline void createColumn(MyObject** var, const char* var_name, double objCoeff, double dualObj,  VarType vartype,
      vector<MyObject*> cons = {}, vector<double> coeffs = {}, bool transformed = false, double score = 0){
      switch(vartype){
      case VARTYPE_BINARY:
         createBinaryColumnVar(var, var_name, objCoeff, dualObj, score);
         break;
      case VARTYPE_INTEGER:
         createIntColumnVar(var, var_name, objCoeff, dualObj, score);
         break;
      default:
         createPositiveColumnVar(var, var_name, objCoeff, dualObj, score);
         break;
      }

      for(int i=0; i<cons.size(); i++)
         addCoefLinear(cons[i], *var, coeffs[i], transformed);
   }

   inline void createPositiveColumn(MyObject** var, const char* var_name, double objCoeff, double dualObj,
      vector<MyObject*> cons = {}, vector<double> coeffs = {}, bool transformed = false, double score = 0){
      createColumn(var, var_name, objCoeff, dualObj, VARTYPE_CONTINUOUS, cons, coeffs, transformed, score);
   }

   inline void createBinaryColumn(MyObject** var, const char* var_name, double objCoeff, double dualObj,
      vector<MyObject*> cons = {}, vector<double> coeffs = {}, bool transformed = false, double score = 0){
      createColumn(var, var_name, objCoeff, dualObj, VARTYPE_BINARY, cons, coeffs, transformed, score);
   }

   inline void createIntColumn(MyObject** var, const char* var_name, double objCoeff, double dualObj,
      vector<MyObject*> cons = {}, vector<double> coeffs = {}, bool transformed = false, double score = 0){
      createColumn(var, var_name, objCoeff, dualObj, VARTYPE_INTEGER, cons, coeffs, transformed, score);
   }

   /*
    * get the primal values
    */

   virtual bool isInteger(MyObject* var){
      double value = getVarValue(var);
      double fractionalPart = round(value) - value;
      if( abs(fractionalPart) < EPSILON )
         return true;
      return false;
   }

   virtual double getVarValue(MyObject* var)=0;

   inline vector<double> getVarValues(vector<MyObject*> vars){
      vector<double> values(vars.size());
      for(int i=0; i<vars.size(); ++i)
         values[i] = getVarValue(vars[i]);
      return values;
   }

   /*
    * Get the dual variables
    */

   virtual double getDual(MyObject* cons, bool transformed = false)=0;

   inline vector<double> getDuals(vector<MyObject*> cons, bool transformed = false){
      vector<double> dualValues(cons.size());
      for(int i=0; i<cons.size(); ++i)
         dualValues[i] = getDual(cons[i], transformed);
      return dualValues;
   }

   /*
    * Getters and setters
    */

   //compute the total cost of MyObject* in the solution
   virtual double getTotalCost(MyObject* var)=0;

   //compute the total cost of a vector of MyObject* in the solution
   template<typename T>  inline double getTotalCost(map<MyObject*, T> map0){
      double value = 0 ;
      for(pair<MyObject*, T> var: map0)
         value += getTotalCost(var.first);
      return value;
   }

   //compute the total cost of a multiple vectors of MyObject* in the solution
   template<typename V> inline double getTotalCost(vector<V> vector){
      double value = 0 ;
      for(V vect: vector)
         value += getTotalCost(vect);
      return value;
   }

   virtual double getObjective()=0;

   /**************
    * Parameters *
    *************/
   virtual int setVerbosity(int v)=0;

   /**************
    * Outputs *
    *************/

   virtual int printStats()=0;

   virtual int printBestSol()=0;

   virtual int writeProblem(string fileName)=0;

   virtual int writeLP(string fileName)=0;

   /**************
    * Getters *
    *************/

   template<typename M> M getModel(){
      string error = "This template has not been implemented.";
      Tools::throwError(error.c_str());
   }

protected:
   //store all MyObject*
   vector<MyObject*> objects_;
};


#endif /* SRC_MODELER_H_ */
