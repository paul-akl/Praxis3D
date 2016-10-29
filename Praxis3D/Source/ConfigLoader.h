#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ErrorCodes.h"
#include "Math.h"
#include "Utilities.h"

// Forward declaration
class NodeIterator;

class ConfigFile
{
friend class NodeIterator;
public:
	ConfigFile();
	~ConfigFile();

	ErrorCode import(std::string p_filename, std::unordered_map<std::string, int> &p_hashMap);
	
	NodeIterator getRootNode();

private:
	// Forward declaration
	class NodeBase;
	class BaseValue;

	// Internal loading functions
	ErrorCode loadFromFile(std::string p_filename);
	ErrorCode generateNodes(std::unordered_map<std::string, int> &p_hashMap);

	// Returns the next string that is between quotation marks (could be a name or a value)
	std::string getNextField(std::vector<std::string>::size_type &p_index);

	// Methods for filling values and nodes, from raw string data
	NodeBase *getNextNode(std::vector<std::string>::size_type &p_index, std::unordered_map<std::string, int> &p_hashMap);

	BaseValue *getValueNode(std::string p_value);

	// All currently supported value types
	class BaseValue
	{
	public:
		virtual std::string getString() = 0;
		virtual bool getBool() = 0;
		virtual int getInt() = 0;
		virtual float getFloat() = 0;
		virtual double getDouble() = 0;
		virtual Math::Vec2f getVec2f() = 0;
		virtual Math::Vec3f getVec3f() = 0;
		virtual Math::Vec4f getVec4f() = 0;

	protected:
		virtual void setParameter(std::string p_value) = 0;
	};
	class BoolValue : public BaseValue
	{
	public:
		BoolValue(std::string p_value)
		{
			setParameter(p_value);
		}

		inline std::string getString()
		{
			return Utilities::toString(m_value);
		}
		inline bool getBool()
		{
			return m_value;
		}
		inline int getInt()
		{
			return m_value ? 0 : 1;
		}
		inline float getFloat()
		{
			return m_value ? 0.0f : 1.0f;
		}
		inline double getDouble()
		{
			return m_value ? 0.0 : 1.0;
		}
		inline Math::Vec2f getVec2f()
		{
			return m_value ? Math::Vec2f(0.0) : Math::Vec2f(1.0);
		}
		inline Math::Vec3f getVec3f()
		{
			return m_value ? Math::Vec3f(0.0) : Math::Vec3f(1.0);
		}
		inline Math::Vec4f getVec4f()
		{
			return m_value ? Math::Vec4f(0.0) : Math::Vec4f(1.0);
		}

	private:
		inline void setParameter(std::string p_value)
		{
			m_value = (p_value == "1" || p_value == "true" || p_value == "True" || p_value == "TRUE");
		}

		bool m_value;
	};
	class IntValue : public BaseValue
	{
	public:
		IntValue(std::string p_value)
		{
			setParameter(p_value);
		}

		inline std::string getString()
		{
			return Utilities::toString(m_value);
		}
		inline bool getBool()
		{
			return m_value == 0 ? false : true;
		}
		inline int getInt()
		{
			return m_value;
		}
		inline float getFloat()
		{
			return (float) m_value;
		}
		inline double getDouble()
		{
			return (double) m_value;
		}
		inline Math::Vec2f getVec2f()
		{
			return Math::Vec2f((float) m_value);
		}
		inline Math::Vec3f getVec3f()
		{
			return Math::Vec3f((float) m_value);
		}
		inline Math::Vec4f getVec4f()
		{
			return Math::Vec4f((float) m_value);
		}

	private:
		inline void setParameter(std::string p_value)
		{
			m_value = std::atoi(p_value.c_str());
		}

		int m_value;
	};
	class FloatValue : public BaseValue
	{
	public:
		FloatValue(std::string p_value)
		{
			setParameter(p_value);
		}

		inline std::string getString()
		{
			return Utilities::toString(m_value);
		}
		inline bool getBool()
		{
			return m_value == 0.0 ? false : true;
		}
		inline int getInt()
		{
			return (int) m_value;
		}
		inline float getFloat()
		{
			return m_value;
		}
		inline double getDouble()
		{
			return (double) m_value;
		}
		inline Math::Vec2f getVec2f()
		{
			return Math::Vec2f(m_value);
		}
		inline Math::Vec3f getVec3f()
		{
			return Math::Vec3f(m_value);
		}
		inline Math::Vec4f getVec4f()
		{
			return Math::Vec4f(m_value);
		}

	private:
		inline void setParameter(std::string p_value)
		{
			m_value = (float) std::atof(p_value.c_str());
		}

		float m_value;
	};
	class DoubleValue : public BaseValue
	{
	public:
		DoubleValue(std::string p_value)
		{
			setParameter(p_value);
		}

		inline std::string getString()
		{
			return Utilities::toString(m_value);
		}
		inline bool getBool()
		{
			return m_value == 0.0 ? false : true;
		}
		inline int getInt()
		{
			return (int) m_value;
		}
		inline float getFloat()
		{
			return (float) m_value;
		}
		inline double getDouble()
		{
			return m_value;
		}
		inline Math::Vec2f getVec2f()
		{
			return Math::Vec2f((float) m_value);
		}
		inline Math::Vec3f getVec3f()
		{
			return Math::Vec3f((float) m_value);
		}
		inline Math::Vec4f getVec4f()
		{
			return Math::Vec4f((float) m_value);
		}

	private:
		inline void setParameter(std::string p_value)
		{
			m_value = std::atof(p_value.c_str());
		}

		double m_value;
	};
	class StringValue : public BaseValue
	{
	public:
		StringValue(std::string p_value)
		{
			setParameter(p_value);
		}

		inline std::string getString()
		{
			return m_value;
		}
		inline bool getBool()
		{
			return std::atoi(m_value.c_str()) == 0 ? true : false;
		}
		inline int getInt()
		{
			return std::atoi(m_value.c_str());
		}
		inline float getFloat()
		{
			return (float) std::atof(m_value.c_str());
		}
		inline double getDouble()
		{
			return std::atof(m_value.c_str());
		}
		inline Math::Vec2f getVec2f()
		{
			return Math::Vec2f((float) std::atof(m_value.c_str()));
		}
		inline Math::Vec3f getVec3f()
		{
			return Math::Vec3f((float) std::atof(m_value.c_str()));
		}
		inline Math::Vec4f getVec4f()
		{
			return Math::Vec4f((float) std::atof(m_value.c_str()));
		}

	private:
		inline void setParameter(std::string p_value)
		{
			m_value = p_value;
		}

		std::string m_value;
	};
	class Vec2fValue : public BaseValue
	{
	public:
		Vec2fValue(std::string p_value)
		{
			setParameter(p_value);
		}

		inline std::string getString()
		{
			return Utilities::toString(m_value.x) + ", " + Utilities::toString(m_value.y);
		}
		inline bool getBool()
		{
			return (m_value.x == 0.0 && m_value.y == 0.0) ? false : true;
		}
		inline int getInt()
		{
			return (int) m_value.x;
		}
		inline float getFloat()
		{
			return m_value.x;
		}
		inline double getDouble()
		{
			return (float) m_value.x;
		}
		inline Math::Vec2f getVec2f()
		{
			return m_value;
		}
		inline Math::Vec3f getVec3f()
		{
			return Math::Vec3f(m_value, 0.0f);
		}
		inline Math::Vec4f getVec4f()
		{
			return Math::Vec4f(m_value, 0.0f, 0.0f);
		}

	private:
		inline void setParameter(std::string p_value)
		{
			std::string xString, yString;
			std::vector<std::string>::size_type index;
			for(index = 0; p_value[index] != ','; index++)
				xString += p_value[index];	index++;
			for(; index < p_value.size(); index++)
				yString += p_value[index];

			m_value = Math::Vec2f((float) std::atof(xString.c_str()), (float) std::atof(yString.c_str()));
		}

		Math::Vec2f m_value;
	};
	class Vec3fValue : public BaseValue
	{
	public:
		Vec3fValue(std::string p_value)
		{
			setParameter(p_value);
		}

		inline std::string getString()
		{
			return Utilities::toString(m_value.x) + ", " + Utilities::toString(m_value.y) + ", " + Utilities::toString(m_value.z);
		}
		inline bool getBool()
		{
			return (m_value.x == 0.0 && m_value.y == 0.0 && m_value.z == 0.0) ? false : true;
		}
		inline int getInt()
		{
			return (int) m_value.x;
		}
		inline float getFloat()
		{
			return m_value.x;
		}
		inline double getDouble()
		{
			return (float) m_value.x;
		}
		inline Math::Vec2f getVec2f()
		{
			return Math::Vec2f(m_value);
		}
		inline Math::Vec3f getVec3f()
		{
			return m_value;
		}
		inline Math::Vec4f getVec4f()
		{
			return Math::Vec4f(m_value, 0.0f);
		}

	private:
		inline void setParameter(std::string p_value)
		{
			std::string xString, yString, zString;
			std::vector<std::string>::size_type index;
			for(index = 0; p_value[index] != ','; index++)
				xString += p_value[index];	index++;
			for(; p_value[index] != ','; index++)
				yString += p_value[index];	index++;
			for(; index < p_value.size(); index++)
				zString += p_value[index];

			m_value = Math::Vec3f((float) std::atof(xString.c_str()),
								  (float) std::atof(yString.c_str()),
								  (float) std::atof(zString.c_str()));
		}

		Math::Vec3f m_value;
	};
	class Vec4fValue : public BaseValue
	{
	public:
		Vec4fValue(std::string p_value)
		{
			setParameter(p_value);
		}

		inline std::string getString()
		{
			return Utilities::toString(m_value.x) + ", " + Utilities::toString(m_value.y) + ", " + Utilities::toString(m_value.z) + ", " + Utilities::toString(m_value.w);
		}
		inline bool getBool()
		{
			return (m_value.x == 0.0 && m_value.y == 0.0 && m_value.z == 0.0 == 0.0 && m_value.w == 0.0) ? false : true;
		}
		inline int getInt()
		{
			return (int) m_value.x;
		}
		inline float getFloat()
		{
			return m_value.x;
		}
		inline double getDouble()
		{
			return (float) m_value.x;
		}
		inline Math::Vec2f getVec2f()
		{
			return Math::Vec2f(m_value);
		}
		inline Math::Vec3f getVec3f()
		{
			return Math::Vec3f(m_value);
		}
		inline Math::Vec4f getVec4f()
		{
			return m_value;
		}

	private:
		inline void setParameter(std::string p_value)
		{
			std::string xString, yString, zString, wString;
			std::vector<std::string>::size_type index;
			for(index = 0; p_value[index] != ','; index++)
				xString += p_value[index];	index++;
			for(; p_value[index] != ','; index++)
				yString += p_value[index];	index++;
			for(; p_value[index] != ','; index++)
				zString += p_value[index];	index++;
			for(; index < p_value.size(); index++)
				wString += p_value[index];
			m_value = Math::Vec4f((float) std::atof(xString.c_str()),
								  (float) std::atof(yString.c_str()),
								  (float) std::atof(zString.c_str()),
								  (float) std::atof(wString.c_str()));
		}

		Math::Vec4f m_value;
	};
	class NullValue : public BaseValue
	{
	public:
		NullValue() { }

		inline std::string getString() { return "null"; }
		inline bool getBool() { return false; }
		inline int getInt() { return 0; }
		inline float getFloat() { return 0.0f; }
		inline double getDouble() { return 0.0; }
		inline Math::Vec2f getVec2f() { return Math::Vec2f(); }
		inline Math::Vec3f getVec3f() { return Math::Vec3f(); }
		inline Math::Vec4f getVec4f() { return Math::Vec4f(); }

	protected:
		inline void setParameter(std::string p_value) { }
	};
	
	class NodeBase
	{
		friend class ConfigFile;
		friend class NodeIterator;
	public:
		NodeBase(std::string p_name, int p_mapKey) : m_name(p_name), m_mapKey(p_mapKey) { }
		~NodeBase() { }

		inline std::string getName() { return m_name; }
		inline int getHashKey() { return m_mapKey; }

		inline bool operator==(const int p_mapKey) { return (m_mapKey == p_mapKey); }
		inline bool operator==(const std::string p_name) { return (m_name == p_name); }

		inline virtual BaseValue &getValue() { return m_nullValue; }

		inline virtual size_t getNumArrayEntries() { return 0; }
		inline virtual size_t getNumChildren() { return 0; }

		inline virtual std::string convertToString(const std::string &p_prefix) { return "\"" + m_name + "\": \"" + m_nullValue.getString() + "\""; }

	protected:
		const inline virtual void setValue(BaseValue *p_value) { delete p_value; }

		inline virtual void addChild(NodeBase *p_child) { delete p_child; }
		inline virtual void addArrayEntry(std::vector<NodeBase*> *p_arrayEntry) { delete p_arrayEntry; }

		inline virtual std::vector<NodeBase*> *getChildren() { return &m_nullArray; }
		inline virtual std::vector<std::vector<ConfigFile::NodeBase*>*> *getNodeArray() { return &m_nullArrayOfArray; }

		int m_mapKey;
		std::string m_name;

		// Null values used for returning instead of null pointers, so the program doesn't crash
		static NullValue m_nullValue;
		static std::vector<NodeBase*> m_nullArray;
		static std::vector<std::vector<ConfigFile::NodeBase*>*> m_nullArrayOfArray;
	};
	class ValueNode : public NodeBase
	{
		friend ConfigFile::NodeBase *ConfigFile::getNextNode(std::vector<std::string>::size_type &p_index, std::unordered_map<std::string, int> &p_hashMap);
	public:
		ValueNode(std::string p_name, int p_mapKey) : NodeBase(p_name, p_mapKey), m_value(nullptr) { }
		~ValueNode() { delete m_value; }

		inline BaseValue &getValue() { return (m_value) ? *m_value : m_nullValue; }

		inline std::string convertToString(const std::string &p_prefix)
		{
			// Add name and value between quotation marks, separate them with colon and a space
			// Also check if value has been defined, if not, use null value
			return p_prefix + "\"" + m_name + "\": \"" + ((m_value != nullptr) ? m_value->getString() : m_nullValue.getString()) + "\"";
		}

	protected:
		const inline void setValue(BaseValue *p_value)
		{
			if(m_value != nullptr)
				delete m_value;
			m_value = p_value;
		}

	private:
		BaseValue *m_value;
	};
	class ParentNode : public NodeBase
	{
		friend ConfigFile::NodeBase *ConfigFile::getNextNode(std::vector<std::string>::size_type &p_index, std::unordered_map<std::string, int> &p_hashMap);
	public:
		ParentNode(std::string p_name, int p_mapKey) : NodeBase(p_name, p_mapKey) { }
		~ParentNode()
		{
			for(std::vector<NodeBase*>::size_type i = 0, vectorSize = m_childNodes.size(); i < vectorSize; i++)
				delete m_childNodes[i];
		}

		inline std::string convertToString(const std::string &p_prefix)
		{
			std::string newPrefix = p_prefix + "\t";
			std::string returnString = p_prefix + "\"" + m_name + "\":\n{\n" + newPrefix;

			// Convert each child to string recursively
			for(std::vector<NodeBase*>::size_type i = 0, size = m_childNodes.size(); i < size; i++)
			{
				returnString += m_childNodes[i]->convertToString(newPrefix);
				if(i + 1 < size)
					returnString += ",\n";
			}

			returnString += "\n}";
			return returnString;
		}

		size_t getNumArrayEntries() { return m_childNodes.size(); }

	protected:
		inline void addChild(NodeBase *p_child) { m_childNodes.push_back(p_child); }

		inline std::vector<NodeBase*> *getChildren() { return &m_childNodes; }

	private:
		std::vector<NodeBase*> m_childNodes;
	};
	class ArrayNode : public NodeBase
	{
		friend ConfigFile::NodeBase *ConfigFile::getNextNode(std::vector<std::string>::size_type &p_index, std::unordered_map<std::string, int> &p_hashMap);
	public:
		ArrayNode(std::string p_name, int p_mapKey) : NodeBase(p_name, p_mapKey), m_nodeArraySize(0) { }
		~ArrayNode()
		{
			for(std::vector<std::vector<NodeBase*>>::size_type i = 0, iSize = m_nodeArray.size(); i < iSize; i++)
			{
				if(m_nodeArray[i] != nullptr)
				{
					for(std::vector<NodeBase*>::size_type j = 0, jSize = m_nodeArray[i]->size(); j < jSize; j++)
						delete (*m_nodeArray[i])[j];
				}
				else
					delete m_nodeArray[i];
			}
		}

		inline std::string convertToString(const std::string &p_prefix)
		{
			std::string outerPrefix = p_prefix + "\t";
			std::string innerPrefix = p_prefix + "\t\t";
			std::string returnString = p_prefix + "\"" + m_name + "\":\n[";

			// Convert each child in each array entry to string recursively
			for(std::vector<std::vector<NodeBase*>*>::size_type i = 0, iSize = m_nodeArray.size(); i < iSize; i++)
			{
				returnString += "\n" + outerPrefix + "{\n";
				for(std::vector<NodeBase*>::size_type j = 0, jSize = m_nodeArray[i]->size(); j < jSize; j++)
				{
					returnString += (*m_nodeArray[i])[j]->convertToString(innerPrefix);
				}
				returnString += "\n" + outerPrefix + "}";
				if(i + 1 < iSize)
					returnString += ",";
			}

			returnString += "\n]";
			return returnString;
		}

		size_t getNumChildren() { return m_nodeArray.size(); }

		//const inline std::vector<NodeBase*> &getArrayEntry(std::vector<std::vector<NodeBase*>*>::size_type p_index) { return (p_index < m_nodeArraySize) ? *m_nodeArray[p_index] : m_nullArray; }

	protected:
		inline void addArrayEntry(std::vector<NodeBase*> *p_arrayEntry)
		{
			m_nodeArray.push_back(p_arrayEntry);
			m_nodeArraySize = m_nodeArray.size();
		}

		inline std::vector<std::vector<ConfigFile::NodeBase*>*> *getNodeArray() { return &m_nodeArray; }

	private:
		std::vector<std::vector<NodeBase*>*> m_nodeArray;
		std::vector<std::vector<NodeBase*>*>::size_type m_nodeArraySize;
	};

	NodeBase *m_rootNode;
	std::string m_filename;
	std::string m_configString;
};

class NodeIterator
{
	friend class ConfigFile;
private:
	NodeIterator()
		: m_nodes(nullptr), m_nodeArray(nullptr), m_endOfArray(true), m_index(0), m_nodeArraySize(0), m_validObject(false)
	{

	}
	NodeIterator(std::vector<ConfigFile::NodeBase*> *p_nodes)
		: m_nodes(p_nodes), m_nodeArray(nullptr), m_endOfArray(false), m_index(0), m_nodeArraySize(0), m_validObject(true)
	{
		if(!p_nodes)
		{
			m_validObject = false;
			m_nodes = nullptr;
		}
		else
			if(p_nodes->size() == 0)
			{
				m_validObject = false;
				m_nodes = nullptr;
			}
	}
	NodeIterator(std::vector<std::vector<ConfigFile::NodeBase*>*> *p_nodeArray)
		: m_nodes(nullptr), m_nodeArray(p_nodeArray), m_endOfArray(false), m_index(0), m_validObject(true)
	{
		if(!p_nodeArray)
		{
			m_nodeArraySize = 0;
			m_validObject = false;
			p_nodeArray = nullptr;
		}
		else
			if(p_nodeArray->size() == 0)
			{
				m_nodeArraySize = 0;
				m_validObject = false;
				p_nodeArray = nullptr;
			}
			else
				m_nodeArraySize = m_nodeArray->size();
	}

	inline ConfigFile::NodeBase *findNode(std::string p_name)
	{
		if(m_nodes)
		{
			for(std::vector<ConfigFile::NodeBase*>::size_type i = 0, size = m_nodes->size(); i < size; i++)
				if(*(*m_nodes)[i] == p_name)
					return (*m_nodes)[i];
		}

		return &m_nullNode;
	}
	inline ConfigFile::NodeBase *findNode(int p_mapKey)
	{
		if(m_nodes)
		{
			for(std::vector<ConfigFile::NodeBase*>::size_type i = 0, size = m_nodes->size(); i < size; i++)
				if(*(*m_nodes)[i] == p_mapKey)
					return (*m_nodes)[i];
		}

		return &m_nullNode;
	}

	std::vector<ConfigFile::NodeBase*> *m_nodes;
	std::vector<std::vector<ConfigFile::NodeBase*>*> *m_nodeArray;
	std::vector<std::vector<ConfigFile::NodeBase*>*>::size_type m_nodeArraySize;

	bool m_endOfArray;
	bool m_validObject;
	unsigned int m_index;

	static ConfigFile::NodeBase m_nullNode;

public:
	~NodeIterator() { }

	inline NodeIterator getNode(std::string p_name)		 { return NodeIterator(findNode(p_name)->getChildren()); }
	inline NodeIterator getNode(int p_mapKey)			 { return NodeIterator(findNode(p_mapKey)->getChildren()); }
	inline NodeIterator getNodeArray(std::string p_name) { return NodeIterator(findNode(p_name)->getNodeArray()); }
	inline NodeIterator getNodeArray(int p_mapKey)		 { return NodeIterator(findNode(p_mapKey)->getNodeArray()); }
	inline NodeIterator getArrayEntry()					 { return (m_nodeArray) ? NodeIterator((*m_nodeArray)[m_index]) : NodeIterator(); }
	
	inline ConfigFile::BaseValue &getValue(std::string p_name) { return findNode(p_name)->getValue(); }
	inline ConfigFile::BaseValue &getValue(int p_mapKey)		{ return findNode(p_mapKey)->getValue(); }

	explicit inline operator bool() const	{ return m_validObject; }
	inline void operator++(int)				{ (m_index < m_nodeArraySize) ? m_index++ : m_endOfArray = true; }
	inline bool endOfArray()				{ return m_endOfArray; }
};