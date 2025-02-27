//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/Options.h>
#include <IceUtil/OutputUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/ConsoleUtil.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include "PHPUtil.h"
#include <Slice/Parser.h>
#include <Slice/Util.h>

#include <cstring>
#include <climits>
#include <mutex>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#else
#  include <unistd.h>
#endif

// TODO: fix this warning!
#if defined(_MSC_VER)
#   pragma warning(disable:4456) // shadow
#   pragma warning(disable:4457) // shadow
#   pragma warning(disable:4459) // shadow
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

using namespace std;
using namespace Slice;
using namespace Slice::PHP;
using namespace IceUtilInternal;

namespace
{

// Get the fully-qualified name of the given definition. If a suffix is provided, it is prepended to the definition's
// unqualified name. If the nameSuffix is provided, it is appended to the container's name.
string
getAbsolute(const ContainedPtr& cont, const string& pfx = std::string(), const string& suffix = std::string())
{
    return scopedToName(cont->scope() + pfx + cont->name() + suffix, true);
}

}

// CodeVisitor generates the PHP mapping for a translation unit.
class CodeVisitor : public ParserVisitor
{
public:

    CodeVisitor(IceUtilInternal::Output&);

    virtual void visitClassDecl(const ClassDeclPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual void visitInterfaceDecl(const InterfaceDeclPtr&);
    virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

private:

    void startNamespace(const ContainedPtr&);
    void endNamespace();

    // Return the PHP name for the given Slice type. When using namespaces, this name is a relative (unqualified) name,
    // otherwise this name is the flattened absolute name.
    string getName(const ContainedPtr&, const string& = string());

    // Return the PHP variable for the given object's type.
    string getTypeVar(const ContainedPtr&, const string& = string());

    // Emit the array for a Slice type.
    void writeType(const TypePtr&);
    string getType(const TypePtr&);

    // Write a default value for a given type.
    void writeDefaultValue(const DataMemberPtr&);

    struct MemberInfo
    {
        string fixedName;
        bool inherited;
        DataMemberPtr dataMember;
    };
    typedef list<MemberInfo> MemberInfoList;

    // Write a member assignment statement for a constructor.
    void writeAssign(const MemberInfo&);

    // Write constant value.
    void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&);

    // Write constructor parameters with default values.
    void writeConstructorParams(const MemberInfoList&);

    // Convert an operation mode into a string.
    string getOperationMode(Slice::Operation::Mode);

    void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
    void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

    Output& _out;
    set<string> _classHistory;
};

// CodeVisitor implementation.
CodeVisitor::CodeVisitor(Output& out) :
    _out(out)
{
}

void
CodeVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    // Handle forward declarations.
    string scoped = p->scoped();
    if(_classHistory.count(scoped) == 0)
    {
        startNamespace(p);

        string type = getTypeVar(p);
        _out << sp << nl << "global " << type << ';';

        _out << nl << "if(!isset(" << type << "))";
        _out << sb;
        _out << nl << type << " = IcePHP_declareClass('" << scoped << "');";
        _out << eb;

        endNamespace();

        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

void
CodeVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    // Handle forward declarations.
    string scoped = p->scoped();
    if(_classHistory.count(scoped) == 0)
    {
        startNamespace(p);

        string type = getTypeVar(p);
        _out << sp << nl << "global " << type << ';';

        _out << nl << "global " << type << "Prx;";
        _out << nl << "if(!isset(" << type << "))";
        _out << sb;
        _out << nl << type << " = IcePHP_declareClass('" << scoped << "');";
        _out << nl << type << "Prx = IcePHP_declareProxy('" << scoped << "');";
        _out << eb;

        endNamespace();

        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

bool
CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p);
    ClassDefPtr base = p->base();
    DataMemberList members = p->dataMembers();

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';

    _out << nl;
    _out << "class " << name;
    if(base)
    {
        _out << " extends " << getAbsolute(base);
    }
    else
    {
        _out << " extends \\Ice\\Value";
    }

    _out << sb;

    // __construct
    _out << nl << "public function __construct(";
    MemberInfoList allMembers;
    collectClassMembers(p, allMembers, false);
    writeConstructorParams(allMembers);
    _out << ")";
    _out << sb;
    if(base)
    {
        _out << nl << "parent::__construct(";
        int count = 0;
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(q->inherited)
            {
                if(count)
                {
                    _out << ", ";
                }
                _out << '$' << q->fixedName;
                ++count;
            }
        }
        _out << ");";
    }
    {
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(!q->inherited)
            {
                writeAssign(*q);
            }
        }
    }
    _out << eb;

    // ice_ice
    _out << sp << nl << "public function ice_id()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    // ice_staticId
    _out << sp << nl << "public static function ice_staticId()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    // __toString
    _out << sp << nl << "public function __toString(): string";

    _out << sb;
    _out << nl << "global " << type << ';';
    _out << nl << "return IcePHP_stringify($this, " << type << ");";
    _out << eb;

    if(!members.empty())
    {
        _out << sp;
        bool isProtected = p->hasMetaData("protected");
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            _out << nl;
            if(isProtected || (*q)->hasMetaData("protected"))
            {
                _out << "protected ";
            }
            else
            {
                _out << "public ";
            }
            _out << "$" << fixIdent((*q)->name()) << ";";
        }
    }

    _out << eb; // End of class.

    if(_classHistory.count(scoped) == 0 && p->canBeCyclic())
    {
        // Emit a forward declaration for the class in case a data member refers to this type.
        _out << sp << nl << type << " = IcePHP_declareClass('" << scoped << "');";
    }

    {
        string type;
        vector<string> seenType;
        _out << sp << nl << "global ";
        if(!base)
        {
            type = "$Ice__t_Value";
        }
        else
        {
            type = getTypeVar(base);
        }
        _out << type << ";";
        seenType.push_back(type);

        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            string type = getType((*q)->type());
            if(find(seenType.begin(), seenType.end(), type) == seenType.end())
            {
                seenType.push_back(type);
                _out << nl << "global " << type << ";";
            }
        }
    }

    // Emit the type information.
    const bool preserved = p->hasMetaData("preserve-slice") || p->inheritsMetaData("preserve-slice");
    _out << nl << type << " = IcePHP_defineClass('" << scoped << "', '" << escapeName(abs) << "', "
         << p->compactId() << ", " << (preserved ? "true" : "false") << ", false, ";
    if(!base)
    {
        _out << "$Ice__t_Value";
    }
    else
    {
        _out << getTypeVar(base);
    }
    _out << ", ";
    // Members
    //
    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    if(!members.empty())
    {
        _out << "array(";
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            if(q != members.begin())
            {
                _out << ',';
            }
            _out.inc();
            _out << nl << "array('" << fixIdent((*q)->name()) << "', ";
            writeType((*q)->type());
            _out << ", " << ((*q)->optional() ? "true" : "false") << ", "
                 << ((*q)->optional() ? (*q)->tag() : 0) << ')';
            _out.dec();
        }
        _out << ')';
    }
    else
    {
        _out << "null";
    }
    _out << ");";

    endNamespace();

    if(_classHistory.count(scoped) == 0)
    {
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }

    return false;
}

bool
CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p);
    string prxName = getName(p, "Prx");
    string prxType = getTypeVar(p, "Prx");
    string prxAbs = getAbsolute(p, "", "Prx");
    InterfaceList bases = p->bases();
    OperationList ops = p->operations();
    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    _out << nl << "global " << prxType << ';';

    // Define the proxy class.
    _out << sp << nl << "class " << prxName << "Helper";
    _out << sb;

    _out << sp << nl << "public static function checkedCast($proxy, $facetOrContext=null, $context=null)";
    _out << sb;
    _out << nl << "return $proxy->ice_checkedCast('" << scoped << "', $facetOrContext, $context);";
    _out << eb;

    _out << sp << nl << "public static function uncheckedCast($proxy, $facet=null)";
    _out << sb;
    _out << nl << "return $proxy->ice_uncheckedCast('" << scoped << "', $facet);";
    _out << eb;

    _out << sp << nl << "public static function ice_staticId()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    _out << eb;

    _out << sp << nl << "global ";
    _out << "$Ice__t_ObjectPrx";
    _out << ";";
    _out << nl << prxType << " = IcePHP_defineProxy('" << scoped << "', ";
    _out << "$Ice__t_ObjectPrx";
    _out << ", ";

    // Interfaces
    if(!bases.empty())
    {
        _out << "array(";
        for(InterfaceList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if(q != bases.begin())
            {
                _out << ", ";
            }
            _out << getTypeVar(*q, "Prx");
        }
        _out << ')';
    }
    else
    {
        _out << "null";
    }
    _out << ");";

    // Define each operation. The arguments to IcePHP_defineOperation are:
    //
    // $ClassType, 'opName', Mode, SendMode, FormatType, (InParams), (OutParams), ReturnParam, (Exceptions)
    //
    // where InParams and OutParams are arrays of type descriptions, and Exceptions
    // is an array of exception type ids.
    if(!ops.empty())
    {
        _out << sp;
        vector<string> seenTypes;
        for(OperationList::const_iterator p = ops.begin(); p != ops.end(); ++p)
        {
            ParamDeclList params = (*p)->parameters();
            for(ParamDeclList::const_iterator q = params.begin(); q != params.end(); ++q)
            {
                string type = getType((*q)->type());
                if(find(seenTypes.begin(), seenTypes.end(), type) == seenTypes.end())
                {
                    seenTypes.push_back(type);
                    _out << nl << "global " << type << ";";
                }
            }

            if((*p)->returnType())
            {
                string type = getType((*p)->returnType());
                if(find(seenTypes.begin(), seenTypes.end(), type) == seenTypes.end())
                {
                    seenTypes.push_back(type);
                    _out << nl << "global " << type << ";";
                }
            }
        }

        for(OperationList::iterator oli = ops.begin(); oli != ops.end(); ++oli)
        {
            ParamDeclList params = (*oli)->parameters();
            ParamDeclList::iterator t;
            int count;

            _out << nl << "IcePHP_defineOperation(" << prxType << ", '" << (*oli)->name() << "', "
                 << getOperationMode((*oli)->mode()) << ", " << getOperationMode((*oli)->sendMode()) << ", "
                 << static_cast<int>((*oli)->format()) << ", ";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if(!(*t)->isOutParam())
                {
                    if(count == 0)
                    {
                        _out << "array(";
                    }
                    else if(count > 0)
                    {
                        _out << ", ";
                    }
                    _out << "array(";
                    writeType((*t)->type());
                    if((*t)->optional())
                    {
                        _out << ", " << (*t)->tag();
                    }
                    _out << ')';
                    ++count;
                }
            }
            if(count > 0)
            {
                _out << ')';
            }
            else
            {
                _out << "null";
            }
            _out << ", ";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if((*t)->isOutParam())
                {
                    if(count == 0)
                    {
                        _out << "array(";
                    }
                    else if(count > 0)
                    {
                        _out << ", ";
                    }
                    _out << "array(";
                    writeType((*t)->type());
                    if((*t)->optional())
                    {
                        _out << ", " << (*t)->tag();
                    }
                    _out << ')';
                    ++count;
                }
            }
            if(count > 0)
            {
                _out << ')';
            }
            else
            {
                _out << "null";
            }
            _out << ", ";
            TypePtr returnType = (*oli)->returnType();
            if(returnType)
            {
                // The return type has the same format as an in/out parameter:
                //
                // Type, Optional?, OptionalTag
                _out << "array(";
                writeType(returnType);
                if((*oli)->returnIsOptional())
                {
                    _out << ", " << (*oli)->returnTag();
                }
                _out << ')';
            }
            else
            {
                _out << "null";
            }
            _out << ", ";
            ExceptionList exceptions = (*oli)->throws();
            if(!exceptions.empty())
            {
                _out << "array(";
                for(ExceptionList::iterator u = exceptions.begin(); u != exceptions.end(); ++u)
                {
                    if(u != exceptions.begin())
                    {
                        _out << ", ";
                    }
                    _out << getTypeVar(*u);
                }
                _out << ')';
            }
            else
            {
                _out << "null";
            }
            _out << ");";
        }
    }

    endNamespace();

    if(_classHistory.count(scoped) == 0)
    {
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }

    return false;
}

bool
CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p);

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    _out << nl << "class " << name << " extends ";
    ExceptionPtr base = p->base();
    string baseName;
    if(base)
    {
        baseName = getAbsolute(base);
        _out << baseName;
    }
    else
    {
        _out << "\\Ice\\UserException";
    }
    _out << sb;

    DataMemberList members = p->dataMembers();

    // __construct
    _out << nl << "public function __construct(";
    MemberInfoList allMembers;
    collectExceptionMembers(p, allMembers, false);
    writeConstructorParams(allMembers);
    _out << ")";
    _out << sb;
    if(base)
    {
        _out << nl << "parent::__construct(";
        int count = 0;
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(q->inherited)
            {
                if(count)
                {
                    _out << ", ";
                }
                _out << '$' << q->fixedName;
                ++count;
            }
        }
        _out << ");";
    }
    for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
    {
        if(!q->inherited)
        {
            writeAssign(*q);
        }
    }
    _out << eb;

    // ice_id
    _out << sp << nl << "public function ice_id()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    // __toString
    _out << sp << nl << "public function __toString(): string";

    _out << sb;
    _out << nl << "global " << type << ';';
    _out << nl << "return IcePHP_stringifyException($this, " << type << ");";
    _out << eb;

    if(!members.empty())
    {
        _out << sp;
        for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
        {
            _out << nl << "public $" << fixIdent((*dmli)->name()) << ";";
        }
    }

    _out << eb;

    vector<string> seenType;
    for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        string type = getType((*dmli)->type());
        if(find(seenType.begin(), seenType.end(), type) == seenType.end())
        {
            seenType.push_back(type);
            _out << nl << "global " << type << ";";
        }
    }

    // Emit the type information.
    const bool preserved = p->hasMetaData("preserve-slice") || p->inheritsMetaData("preserve-slice");
    _out << sp << nl << type << " = IcePHP_defineException('" << scoped << "', '" << escapeName(abs) << "', "
         << (preserved ? "true" : "false") << ", ";
    if(!base)
    {
        _out << "null";
    }
    else
    {
        _out << getTypeVar(base);
    }
    _out << ", ";
    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    if(!members.empty())
    {
        _out << "array(";
        for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
        {
            if(dmli != members.begin())
            {
                _out << ',';
            }
            _out.inc();
            _out << nl << "array('" << fixIdent((*dmli)->name()) << "', ";
            writeType((*dmli)->type());
            _out << ", " << ((*dmli)->optional() ? "true" : "false") << ", "
                 << ((*dmli)->optional() ? (*dmli)->tag() : 0) << ')';
            _out.dec();
        }
        _out << ')';
    }
    else
    {
        _out << "null";
    }
    _out << ");";

    endNamespace();

    return false;
}

bool
CodeVisitor::visitStructStart(const StructPtr& p)
{
    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p);
    MemberInfoList memberList;

    {
        DataMemberList members = p->dataMembers();
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            memberList.push_back(MemberInfo());
            memberList.back().fixedName = fixIdent((*q)->name());
            memberList.back().inherited = false;
            memberList.back().dataMember = *q;
        }
    }

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';

    _out << nl << "class " << name;
    _out << sb;
    _out << nl << "public function __construct(";
    writeConstructorParams(memberList);
    _out << ")";
    _out << sb;
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        writeAssign(*r);
    }
    _out << eb;

    // __toString
    _out << sp << nl << "public function __toString(): string";

    _out << sb;
    _out << nl << "global " << type << ';';
    _out << nl << "return IcePHP_stringify($this, " << type << ");";
    _out << eb;

    if(!memberList.empty())
    {
        _out << sp;
        for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
        {
            _out << nl << "public $" << r->fixedName << ';';
        }
    }

    _out << eb;

    _out << sp;
    vector<string> seenType;
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        string type = getType(r->dataMember->type());
        if(find(seenType.begin(), seenType.end(), type) == seenType.end())
        {
            seenType.push_back(type);
            _out << nl << "global " << type << ";";
        }
    }

    // Emit the type information.
    _out << nl << type << " = IcePHP_defineStruct('" << scoped << "', '" << escapeName(abs) << "', array(";

    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        if(r != memberList.begin())
        {
            _out << ",";
        }
        _out.inc();
        _out << nl << "array('" << r->fixedName << "', ";
        writeType(r->dataMember->type());
        _out << ')';
        _out.dec();
    }
    _out << "));";

    endNamespace();

    return false;
}

void
CodeVisitor::visitSequence(const SequencePtr& p)
{
    string type = getTypeVar(p);
    TypePtr content = p->type();

    startNamespace(p);

    // Emit the type information.
    string scoped = p->scoped();
    _out << sp << nl << "global " << type << ';';
    _out << sp << nl << "if(!isset(" << type << "))";
    _out << sb;
    _out << nl << "global " << getType(content) << ";";
    _out << nl << type << " = IcePHP_defineSequence('" << scoped << "', ";
    writeType(content);
    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    TypePtr keyType = p->keyType();
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(keyType);

    const UnitPtr unit = p->unit();
    const DefinitionContextPtr dc = unit->findDefinitionContext(p->file());
    assert(dc);
    if(b)
    {
        switch(b->kind())
        {
            case Slice::Builtin::KindBool:
            case Slice::Builtin::KindByte:
            case Slice::Builtin::KindShort:
            case Slice::Builtin::KindInt:
            case Slice::Builtin::KindLong:
            case Slice::Builtin::KindString:
                // These types are acceptable as dictionary keys.
                break;

            case Slice::Builtin::KindFloat:
            case Slice::Builtin::KindDouble:
            {
                dc->warning(InvalidMetaData, p->file(), p->line(), "dictionary key type not supported in PHP");
                break;
            }

            case Slice::Builtin::KindObject:
            case Slice::Builtin::KindObjectProxy:
            case Slice::Builtin::KindValue:
                assert(false);
        }
    }
    else if(!dynamic_pointer_cast<Enum>(keyType))
    {
        dc->warning(InvalidMetaData, p->file(), p->line(), "dictionary key type not supported in PHP");
    }

    string type = getTypeVar(p);

    startNamespace(p);

    // Emit the type information.
    string scoped = p->scoped();
    _out << sp << nl << "global " << type << ';';
    _out << sp << nl << "if(!isset(" << type << "))";
    _out << sb;
    _out << nl << "global " << getType(p->keyType()) << ";";
    _out << nl << "global " << getType(p->valueType()) << ";";
    _out << nl << type << " = IcePHP_defineDictionary('" << scoped << "', ";
    writeType(p->keyType());
    _out << ", ";
    writeType(p->valueType());
    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::visitEnum(const EnumPtr& p)
{
    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p);
    EnumeratorList enums = p->enumerators();

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    _out << nl << "class " << name;
    _out << sb;

    {
        long i = 0;
        for(EnumeratorList::iterator q = enums.begin(); q != enums.end(); ++q, ++i)
        {
            _out << nl << "const " << fixIdent((*q)->name()) << " = " << (*q)->value() << ';';
        }
    }

    _out << eb;

    // Emit the type information.
    _out << sp << nl << type << " = IcePHP_defineEnum('" << scoped << "', array(";
    for(EnumeratorList::iterator q = enums.begin(); q != enums.end(); ++q)
    {
        if(q != enums.begin())
        {
            _out << ", ";
        }
        _out << "'" << (*q)->name() << "', " << (*q)->value();
    }
    _out << "));";

    endNamespace();
}

void
CodeVisitor::visitConst(const ConstPtr& p)
{
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p);

    startNamespace(p);

    _out << sp << nl << "if(!defined('" << escapeName(abs) << "'))";
    _out << sb;
    _out << sp << nl << "define(__NAMESPACE__ . '\\\\" << name << "', ";
    writeConstantValue(p->type(), p->valueType(), p->value());

    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::startNamespace(const ContainedPtr& cont)
{
    string scope = cont->scope();
    scope = scope.substr(2); // Removing leading '::'
    scope = scope.substr(0, scope.length() - 2); // Removing trailing '::'
    _out << sp << nl << "namespace " << scopedToName(scope, true);
    _out << sb;
}

void
CodeVisitor::endNamespace()
{
    _out << eb;
}

string
CodeVisitor::getTypeVar(const ContainedPtr& p, const string& suffix)
{
    return "$" + scopedToName(p->scope() + "_t_" + p->name() + suffix, false);
}

string
CodeVisitor::getName(const ContainedPtr& p, const string& suffix)
{
    return fixIdent(p->name() + suffix);
}

void
CodeVisitor::writeType(const TypePtr& p)
{
    _out << getType(p);
}

string
CodeVisitor::getType(const TypePtr& p)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "$IcePHP__t_bool";
            }
            case Builtin::KindByte:
            {
                return "$IcePHP__t_byte";
            }
            case Builtin::KindShort:
            {
                return "$IcePHP__t_short";
            }
            case Builtin::KindInt:
            {
                return "$IcePHP__t_int";
            }
            case Builtin::KindLong:
            {
                return "$IcePHP__t_long";
            }
            case Builtin::KindFloat:
            {
                return "$IcePHP__t_float";
            }
            case Builtin::KindDouble:
            {
                return "$IcePHP__t_double";
            }
            case Builtin::KindString:
            {
                return "$IcePHP__t_string";
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                return "$Ice__t_Value";
            }
            case Builtin::KindObjectProxy:
            {
                return "$Ice__t_ObjectPrx";
            }
        }
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(p);
    if(prx)
    {
        return getTypeVar(prx, "Prx");
    }

    ContainedPtr cont = dynamic_pointer_cast<Contained>(p);
    assert(cont);
    return getTypeVar(cont);
}

void
CodeVisitor::writeDefaultValue(const DataMemberPtr& m)
{
    TypePtr p = m->type();
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "false";
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                _out << "0";
                break;
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                _out << "0.0";
                break;
            }
            case Builtin::KindString:
            {
                _out << "''";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                _out << "null";
                break;
            }
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(p);
    if(en)
    {
        EnumeratorList enums = en->enumerators();
        _out << getAbsolute(en) << "::" << fixIdent(enums.front()->name());
        return;
    }

    // PHP does not allow the following construct:
    //
    // function foo($theStruct=new MyStructType)
    //
    // Instead we use null as the default value and allocate an instance in the constructor.
    if(dynamic_pointer_cast<Struct>(p))
    {
        _out << "null";
        return;
    }

    _out << "null";
}

void
CodeVisitor::writeAssign(const MemberInfo& info)
{
    StructPtr st = dynamic_pointer_cast<Struct>(info.dataMember->type());
    if(st)
    {
        _out << nl << "$this->" << info.fixedName << " = is_null($" << info.fixedName << ") ? new "
             << getAbsolute(st) << " : $" << info.fixedName << ';';
    }
    else
    {
        _out << nl << "$this->" << info.fixedName << " = $" << info.fixedName << ';';
    }
}

void
CodeVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if(constant)
    {
        _out << getAbsolute(constant);
    }
    else
    {
        Slice::BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
        Slice::EnumPtr en = dynamic_pointer_cast<Enum>(type);
        if(b)
        {
            switch(b->kind())
            {
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                {
                    _out << value;
                    break;
                }
                case Slice::Builtin::KindLong:
                {
                    IceUtil::Int64 l;
                    IceUtilInternal::stringToInt64(value, l);
                    // The platform's 'long' type may not be 64 bits, so we store 64-bit values as a string.
                    if(sizeof(IceUtil::Int64) > sizeof(long) && (l < LONG_MIN || l > LONG_MAX))
                    {
                        _out << "'" << value << "'";
                    }
                    else
                    {
                        _out << value;
                    }
                    break;
                }
                case Slice::Builtin::KindString:
                {
                    // PHP 7.x also supports an EC6UCN-like notation, see: https://wiki.php.net/rfc/unicode_escape
                    _out << "\"" << toStringLiteral(value, "\f\n\r\t\v\x1b", "$", Octal, 0) << "\"";
                    break;
                }
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindObjectProxy:
                case Slice::Builtin::KindValue:
                    assert(false);
            }
        }
        else if(en)
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
            assert(lte);
            _out << getAbsolute(en) << "::" << fixIdent(lte->name());
        }
        else
        {
            assert(false); // Unknown const type.
        }
    }
}

void
CodeVisitor::writeConstructorParams(const MemberInfoList& members)
{
    for(MemberInfoList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if(p != members.begin())
        {
            _out << ", ";
        }
        _out << '$' << p->fixedName << "=";

        const DataMemberPtr member = p->dataMember;
        if(member->defaultValueType())
        {
            writeConstantValue(member->type(), member->defaultValueType(), member->defaultValue());
        }
        else if(member->optional())
        {
            _out << "\\Ice\\None";
        }
        else
        {
            writeDefaultValue(member);
        }
    }
}

string
CodeVisitor::getOperationMode(Slice::Operation::Mode mode)
{
    ostringstream ostr;
    ostr << static_cast<int>(mode);
    return ostr.str();
}

void
CodeVisitor::collectClassMembers(const ClassDefPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ClassDefPtr base = p->base();
    if(base)
    {
        collectClassMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MemberInfo m;
        m.fixedName = fixIdent((*q)->name());
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

void
CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ExceptionPtr base = p->base();
    if(base)
    {
        collectExceptionMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MemberInfo m;
        m.fixedName = fixIdent((*q)->name());
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

static void
generate(const UnitPtr& un, bool all, const vector<string>& includePaths, Output& out)
{
    if(!all)
    {
        vector<string> paths = includePaths;
        for(vector<string>::iterator p = paths.begin(); p != paths.end(); ++p)
        {
            *p = fullPath(*p);
        }

        StringList includes = un->includeFiles();
        if(!includes.empty())
        {
            out << sp;
            out << nl << "namespace";
            out << sb;
            for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
            {
                string file = changeInclude(*q, paths);
                out << nl << "require_once '" << file << ".php';";
            }
            out << eb;
        }
    }

    CodeVisitor codeVisitor(out);
    un->visit(&codeVisitor, false);

    out << nl; // Trailing newline.
}

static void
printHeader(IceUtilInternal::Output& out)
{
    static const char* header =
        "//\n"
        "// Copyright (c) ZeroC, Inc. All rights reserved.\n"
        "//\n"
        ;

    out << header;
    out << "//\n";
    out << "// Ice version " << ICE_STRING_VERSION << "\n";
    out << "//\n";
}

namespace
{

mutex globalMutex;
bool interrupted = false;

}

static void
interruptedCallback(int /*signal*/)
{
    lock_guard lock(globalMutex);
    interrupted = true;
}

static void
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr <<
        "Options:\n"
        "-h, --help               Show this message.\n"
        "-v, --version            Display the Ice version.\n"
        "-DNAME                   Define NAME as 1.\n"
        "-DNAME=DEF               Define NAME as DEF.\n"
        "-UNAME                   Remove any definition for NAME.\n"
        "-IDIR                    Put DIR in the include file search path.\n"
        "-E                       Print preprocessor output on stdout.\n"
        "--output-dir DIR         Create files in the directory DIR.\n"
        "-d, --debug              Print debug messages.\n"
        "--depend                 Generate Makefile dependencies.\n"
        "--depend-xml             Generate dependencies in XML format.\n"
        "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
        "--validate               Validate command line options.\n"
        "--all                    Generate code for Slice definitions in included files.\n"
        ;
}

int
compile(const vector<string>& argv)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceUtilInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "all");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();

    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.reason << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-D" + *i);
    }

    optargs = opts.argVec("U");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-U" + *i);
    }

    vector<string> includePaths = opts.argVec("I");
    for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool all = opts.isSet("all");

    if(args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(validate)
    {
        return EXIT_SUCCESS;
    }

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if(dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        // Ignore duplicates.
        vector<string>::iterator p = find(args.begin(), args.end(), *i);
        if(p != i)
        {
            continue;
        }

        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PHP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit(false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if(parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(os, depend ? Preprocessor::PHP : Preprocessor::SliceXML,
                                                  includePaths, "-D__SLICE2PHP__"))
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PHP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            if(preprocess)
            {
                char buf[4096];
                while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != nullptr)
                {
                    if(fputs(buf, stdout) == EOF)
                    {
                        return EXIT_FAILURE;
                    }
                }
                if(!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit(all);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if(!icecpp->close())
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }

                if(parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    string base = icecpp->getBaseName();
                    string::size_type pos = base.find_last_of("/\\");
                    if(pos != string::npos)
                    {
                        base.erase(0, pos + 1);
                    }

                    string file = base + ".php";
                    if(!output.empty())
                    {
                        file = output + '/' + file;
                    }

                    try
                    {
                        IceUtilInternal::Output out;
                        out.open(file.c_str());
                        if(!out)
                        {
                            ostringstream os;
                            os << "cannot open`" << file << "': " << IceUtilInternal::errorToString(errno);
                            throw FileException(__FILE__, __LINE__, os.str());
                        }
                        FileTracker::instance()->addFile(file);

                        out << "<?php\n";
                        printHeader(out);
                        printGeneratedHeader(out, base + ".ice");

                        // Generate the PHP mapping.
                        generate(u, all, includePaths, out);

                        out << "?>\n";
                        out.close();
                    }
                    catch(const Slice::FileException& ex)
                    {
                        // If a file could not be created, then cleanup any  created files.
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.reason() << endl;
                        return EXIT_FAILURE;
                    }
                    catch(const string& err)
                    {
                        FileTracker::instance()->cleanup();
                        consoleErr << argv[0] << ": error: " << err << endl;
                        status = EXIT_FAILURE;
                    }
                }

                u->destroy();
            }
        }

        {
            lock_guard lock(globalMutex);
            if(interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if(dependxml)
    {
        os << "</dependencies>\n";
    }

    if(depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    return status;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        consoleErr << args[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
