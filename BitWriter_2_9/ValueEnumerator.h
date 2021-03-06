//===-- Bitcode/Writer/ValueEnumerator.h - Number values --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class gives values and types Unique ID's.
//
//===----------------------------------------------------------------------===//

#ifndef VALUE_ENUMERATOR_H
#define VALUE_ENUMERATOR_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Attributes.h"
#include <vector>

namespace llvm {

class Type;
class Value;
class Instruction;
class BasicBlock;
class Function;
class Module;
class MDNode;
class NamedMDNode;
class AttrListPtr;
class ValueSymbolTable;
class MDSymbolTable;
class raw_ostream;

}  // end llvm namespace

namespace llvm_2_9 {

class ValueEnumerator {
public:
  typedef std::vector<llvm::Type*> TypeList;

  // For each value, we remember its Value* and occurrence frequency.
  typedef std::vector<std::pair<const llvm::Value*, unsigned> > ValueList;
private:
  typedef llvm::DenseMap<llvm::Type*, unsigned> TypeMapType;
  TypeMapType TypeMap;
  TypeList Types;

  typedef llvm::DenseMap<const llvm::Value*, unsigned> ValueMapType;
  ValueMapType ValueMap;
  ValueList Values;
  ValueList MDValues;
  llvm::SmallVector<const llvm::MDNode *, 8> FunctionLocalMDs;
  ValueMapType MDValueMap;
  
  typedef llvm::DenseMap<void*, unsigned> AttributeMapType;
  AttributeMapType AttributeMap;
  std::vector<llvm::AttrListPtr> Attributes;
  
  /// GlobalBasicBlockIDs - This map memoizes the basic block ID's referenced by
  /// the "getGlobalBasicBlockID" method.
  mutable llvm::DenseMap<const llvm::BasicBlock*, unsigned> GlobalBasicBlockIDs;
  
  typedef llvm::DenseMap<const llvm::Instruction*, unsigned> InstructionMapType;
  InstructionMapType InstructionMap;
  unsigned InstructionCount;

  /// BasicBlocks - This contains all the basic blocks for the currently
  /// incorporated function.  Their reverse mapping is stored in ValueMap.
  std::vector<const llvm::BasicBlock*> BasicBlocks;
  
  /// When a function is incorporated, this is the size of the Values list
  /// before incorporation.
  unsigned NumModuleValues;

  /// When a function is incorporated, this is the size of the MDValues list
  /// before incorporation.
  unsigned NumModuleMDValues;

  unsigned FirstFuncConstantID;
  unsigned FirstInstID;
  
  ValueEnumerator(const ValueEnumerator &);  // DO NOT IMPLEMENT
  void operator=(const ValueEnumerator &);   // DO NOT IMPLEMENT
public:
  ValueEnumerator(const llvm::Module *M);

  void dump() const;
  void print(llvm::raw_ostream &OS, const ValueMapType &Map, const char *Name) const;

  unsigned getValueID(const llvm::Value *V) const;

  unsigned getTypeID(llvm::Type *T) const {
    TypeMapType::const_iterator I = TypeMap.find(T);
    assert(I != TypeMap.end() && "Type not in ValueEnumerator!");
    return I->second-1;
  }

  unsigned getInstructionID(const llvm::Instruction *I) const;
  void setInstructionID(const llvm::Instruction *I);

  unsigned getAttributeID(const llvm::AttrListPtr &PAL) const {
    if (PAL.isEmpty()) return 0;  // Null maps to zero.
    AttributeMapType::const_iterator I = AttributeMap.find(PAL.getRawPointer());
    assert(I != AttributeMap.end() && "Attribute not in ValueEnumerator!");
    return I->second;
  }

  /// getFunctionConstantRange - Return the range of values that corresponds to
  /// function-local constants.
  void getFunctionConstantRange(unsigned &Start, unsigned &End) const {
    Start = FirstFuncConstantID;
    End = FirstInstID;
  }
  
  const ValueList &getValues() const { return Values; }
  const ValueList &getMDValues() const { return MDValues; }
  const llvm::SmallVector<const llvm::MDNode *, 8> &getFunctionLocalMDValues() const { 
    return FunctionLocalMDs;
  }
  const TypeList &getTypes() const { return Types; }
  const std::vector<const llvm::BasicBlock*> &getBasicBlocks() const {
    return BasicBlocks; 
  }
  const std::vector<llvm::AttrListPtr> &getAttributes() const {
    return Attributes;
  }
  
  /// getGlobalBasicBlockID - This returns the function-specific ID for the
  /// specified basic block.  This is relatively expensive information, so it
  /// should only be used by rare constructs such as address-of-label.
  unsigned getGlobalBasicBlockID(const llvm::BasicBlock *BB) const;

  /// incorporateFunction/purgeFunction - If you'd like to deal with a function,
  /// use these two methods to get its data into the ValueEnumerator!
  ///
  void incorporateFunction(const llvm::Function &F);
  void purgeFunction();

private:
  void OptimizeConstants(unsigned CstStart, unsigned CstEnd);
    
  void EnumerateMDNodeOperands(const llvm::MDNode *N);
  void EnumerateMetadata(const llvm::Value *MD);
  void EnumerateFunctionLocalMetadata(const llvm::MDNode *N);
  void EnumerateNamedMDNode(const llvm::NamedMDNode *NMD);
  void EnumerateValue(const llvm::Value *V);
  void EnumerateType(llvm::Type *T);
  void EnumerateOperandType(const llvm::Value *V);
  void EnumerateAttributes(const llvm::AttrListPtr &PAL);
  
  void EnumerateValueSymbolTable(const llvm::ValueSymbolTable &ST);
  void EnumerateNamedMetadata(const llvm::Module *M);
};

}  // end llvm_2_9 namespace

#endif
