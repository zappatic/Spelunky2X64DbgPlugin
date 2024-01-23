#include "Data/CPPGenerator.h"
#include "Configuration.h"
#include "QtHelpers/CPPSyntaxHighlighter.h"

void S2Plugin::CPPGenerator::generate(const std::string& typeName, CPPSyntaxHighlighter* highlighter)
{
    std::string className = typeName;

    std::vector<std::string> dependencies;
    std::string parentClassName = "";
    std::vector<MemoryField> fields;
    auto config = Configuration::get();
    if (config->isEntitySubclass(className))
    {
        const auto& hierarchy = config->entityClassHierarchy();
        if (hierarchy.count(className) > 0)
        {
            parentClassName = hierarchy.at(className);
        }
        fields = config->typeFieldsOfEntitySubclass(className);

        // add the parents to the dependencies
        std::string p = parentClassName;
        while (p != "Entity" && p != "")
        {
            dependencies.emplace_back(p);
            p = hierarchy.at(p);
        }
        if (parentClassName != "")
        {
            dependencies.emplace_back("Entity");
        }
    }
    else if (auto& vec = config->typeFieldsOfDefaultStruct(className); !vec.empty())
    {
        fields = vec;
    }
    else
    {
        mSS << "generate() called for unknown type: " << className;
        return;
    }

    QString qClassName = QString("\\b" + QRegularExpression::escape(QString::fromStdString(className)) + "\\b");
    highlighter->addRule(qClassName, HighlightColor::Type);

    auto skipCounter = 1;
    mGeneratedTypes.insert(typeName);
    mSS << "class " << className;
    if (!parentClassName.empty())
    {
        mSS << " : public " << parentClassName;
    }
    mSS << "\n";
    mSS << "{\n";
    mSS << "\tpublic:\n";
    for (const auto& field : fields)
    {
        mSS << "\t\t";

        std::string variableType;
        std::string variableName = field.name;

        if (field.type == MemoryFieldType::Skip)
        {
            variableType = "uint8_t";
            variableName = "skip[" + std::to_string(field.size) + "]";
        }
        else if (auto str = Configuration::getCPPTypeName(field.type); !str.empty())
        {
            variableType = str;
        }
        else if (field.isPointer)
        {
            std::string pointerLessFieldType = field.jsonName;
            auto pointerIndex = pointerLessFieldType.find("Pointer");
            if (pointerIndex != std::string::npos)
            {
                pointerLessFieldType.replace(pointerIndex, 7, "");
            }
            variableType = pointerLessFieldType + "*";
            dependencies.emplace_back(field.jsonName);
        }
        else if (field.type == MemoryFieldType::DefaultStructType)
        {
            variableType = field.jsonName;
            dependencies.emplace_back(field.jsonName);
        }
        else
        {
            mSS << "unknown field type " << (uint32_t)(field.type);
            variableType = "???";
        }

        mSS << variableType << " " << variableName << ";";
        if (!field.comment.empty())
        {
            mSS << " // " << field.comment;
        }
        mSS << "\n";

        QString qVariableType = QString("\\b" + QRegularExpression::escape(QString::fromStdString(variableType)) + "\\s");
        QString qVariableName = QString("\\b" + QRegularExpression::escape(QString::fromStdString(variableName)) + "[\\s;]");
        highlighter->addRule(qVariableType, HighlightColor::Type);
        highlighter->addRule(qVariableName, HighlightColor::Variable);
    }
    mSS << "};\n\n";

    for (const auto& dep : dependencies)
    {
        if (mGeneratedTypes.count(dep) == 0)
        {
            generate(dep, highlighter);
        }
    }
}

std::string S2Plugin::CPPGenerator::result() const
{
    return mSS.str();
}
