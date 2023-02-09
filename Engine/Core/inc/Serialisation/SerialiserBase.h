#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include "Core/Memory.h"

#include "Serialisation/Serialisers/ISerialiser.h"

#include <vector>

namespace Insight
{
    namespace Serialisation
    {
        class ISerialiser;

        enum class SerialiserType
        {
            Property,
            Object,
            Base,
            VectorProperty,
            VectorObject,
            UMapProperty,
            UMapObject,
        };

        template<typename TypeSerialiser, typename T>
        void SerialiseProperty(ISerialiser* serialiser, std::string_view propertyName, T& data)
        {
            ::Insight::Serialisation::PropertySerialiser<TypeSerialiser> propertySerialiser;
            auto SerialisedData = propertySerialiser(data);
            serialiser->Write(propertyName, SerialisedData);
        }

        template<typename Type>
        void SerialiseObject(ISerialiser* serialiser, Type& data)
        {
            ::Insight::Serialisation::SerialiserObject<Type> objectSerialiser;
            objectSerialiser.Serialise(data, serialiser);
        }

        template<typename Type>
        void SerialiseObject(ISerialiser* serialiser, Type* data)
        {
            ::Insight::Serialisation::SerialiserObject<Type> objectSerialiser;
            objectSerialiser.Serialise(data, serialiser);
        }

        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct SerialiserObject
        {
            void Serialise(T& object, ISerialiser* serialiser)
            {
                assert(false);
            }
            void Serialise(T* object, ISerialiser* serialiser)
            {
                assert(false);
            }
        };

        template<typename T, typename TypeSerialiser, SerialiserType SerialiserType>
        struct VectorSerialiser
        {
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<T>& object)
            {
                if (!serialiser)
                {
                    return;
                }

                serialiser->Write(std::string(name) + c_ArraySize, object.size());
                serialiser->StartArray(name);
                for (auto& v : object)
                {
                    if constexpr (is_insight_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            ::Insight::Serialisation::PropertySerialiser<TypeSerialiser> propertySerialiser;

                            using TVectorElementType = std::remove_pointer_t<decltype(v.Get())>;
                            static_assert(std::is_same_v<TVectorElementType, ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");

                            auto SerialisedData = propertySerialiser(*v.Get());
                            serialiser->Write("", SerialisedData);
                        }
                        else
                        {
                            ::Insight::Serialisation::SerialiserObject<TypeSerialiser> serialiserObject;

                            using TVectorElementType = std::remove_pointer_t<decltype(v.Get())>;
                            static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");

                            serialiserObject.Serialise(*v.Get(), serialiser);
                        }
                    }
                    else if constexpr (is_stl_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            ::Insight::Serialisation::PropertySerialiser<TypeSerialiser> propertySerialiser;

                            using TVectorElementType = std::remove_pointer_t<decltype(v.get())>;
                            static_assert(std::is_same_v<TVectorElementType, ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");

                            auto SerialisedData = propertySerialiser(*v.get());
                            serialiser->Write("", SerialisedData);
                        }
                        else
                        {
                            ::Insight::Serialisation::SerialiserObject<TypeSerialiser> serialiserObject;

                            using TVectorElementType = std::remove_pointer_t<decltype(v.get())>;
                            static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");
                            serialiserObject.Serialise(*v.get(), serialiser);

                        }
                    }
                    else
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            ::Insight::Serialisation::PropertySerialiser<TypeSerialiser> propertySerialiser;
                            auto SerialisedData = propertySerialiser(v);
                            serialiser->Write("", SerialisedData);
                        }
                        else if (SerialiserType == SerialiserType::VectorObject)
                        {
                            ::Insight::Serialisation::SerialiserObject<TypeSerialiser> serialiserObject;
                            serialiserObject.Serialise(v, serialiser);
                        }
                    }
                }
                serialiser->StopArray();
            }
        };

        template<typename T, typename TypeSerialiser, SerialiserType SerialiserType>
        struct VectorDeserialiser
        {
            void operator()(ISerialiser* serialiser, std::string_view name, std::vector<T>& object)
            {
                if (!serialiser)
                {
                    return;
                }

                u64 arraySize = 0;
                serialiser->Read(std::string(name) + c_ArraySize, arraySize);
                serialiser->StartArray(name);
                for (auto& v : object)
                {
                    if constexpr (is_insight_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            using TVectorElementType = std::remove_pointer_t<decltype(v.Get())>;
                            static_assert(std::is_same_v<TVectorElementType, ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");

                            ::Insight::Serialisation::SerialiseProperty(serialiser, name, *v.Get());
                        }
                        else
                        {
                            ::Insight::Serialisation::SerialiserObject<TypeSerialiser> serialiserObject;

                            using TVectorElementType = std::remove_pointer_t<decltype(v.Get())>;
                            static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");

                            serialiserObject.Serialise(*v.Get(), serialiser);
                        }
                    }
                    else if constexpr (is_stl_smart_pointer_v<T>)
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            using TVectorElementType = std::remove_pointer_t<decltype(v.get())>;
                            static_assert(std::is_same_v<TVectorElementType, ::Insight::Serialisation::PropertySerialiser<TypeSerialiser>::InType>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser::InType.");

                            ::Insight::Serialisation::SerialiseProperty(serialiser, name, *v.get());
                        }
                        else
                        {
                            ::Insight::Serialisation::SerialiserObject<TypeSerialiser> serialiserObject;

                            using TVectorElementType = std::remove_pointer_t<decltype(v.get())>;
                            static_assert(std::is_same_v<TVectorElementType, TypeSerialiser>,
                                "[VectorSerialiser] TVectorElementType is different from TypeSerialiser. Did you mean to use PropertySerialiser?");
                            serialiserObject.Serialise(*v.get(), serialiser);

                        }
                    }
                    else
                    {
                        if constexpr (SerialiserType == SerialiserType::VectorProperty)
                        {
                            ::Insight::Serialisation::SerialiseProperty(serialiser, name, v);
                        }
                        else if (SerialiserType == SerialiserType::VectorObject)
                        {
                            ::Insight::Serialisation::SerialiserObject<TypeSerialiser> serialiserObject;
                            serialiserObject.Serialise(v, serialiser);
                        }
                    }
                }
                serialiser->StopArray();
            }
        };

        template<typename TypeSerialiser, typename T, typename TObjectType>
        struct ComplexSerialiser
        {
            void operator()(T const& v, TObjectType* object, ISerialiser* serialiser) const
            {
                assert(false);
            }
        };

        //template<typename TKey, typename TValue>
        //struct UMapSerialiser
        //{
        //    nlohmann::json operator()(std::unordered_map<TKey, TValue> const& object)
        //    {
        //        using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<T>>>;
        //        constexpr bool c_KeyIsFundemental = std::is_fundamental_v<TKey>;
        //        constexpr bool c_ValueIsFundemental = std::is_fundamental_v<TValue>;

        //        if constexpr (ObjectSerialiser)
        //        {
        //            nlohmann::json json;
        //            //SerialiserObject<Type> objectSerialiser;
        //            //for (size_t i = 0; i < object.size(); ++i)
        //            //{
        //            //    if constexpr (std::is_pointer_v<T>)
        //            //    {
        //            //        json.push_back(objectSerialiser.SerialiseToJsonObject(*object.at(i)));
        //            //    }
        //            //    else
        //            //    {
        //            //        json.push_back(objectSerialiser.SerialiseToJsonObject(object.at(i)));
        //            //    }
        //            //}
        //            return json;
        //        }
        //    }
        //};

        //template<typename TKey, typename TValue>
        //struct UMapSerialiser
        //{
        //    nlohmann::json operator()(std::unordered_map<TKey, TValue> const& object)
        //    {
        //        using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<T>>>;
        //        constexpr bool c_KeyIsFundemental = std::is_fundamental_v<TKey>;
        //        constexpr bool c_ValueIsFundemental = std::is_fundamental_v<TValue>;

        //        if constexpr (ObjectSerialiser)
        //        {
        //            nlohmann::json json;
        //            //SerialiserObject<Type> objectSerialiser;
        //            //for (size_t i = 0; i < object.size(); ++i)
        //            //{
        //            //    if constexpr (std::is_pointer_v<T>)
        //            //    {
        //            //        json.push_back(objectSerialiser.SerialiseToJsonObject(*object.at(i)));
        //            //    }
        //            //    else
        //            //    {
        //            //        json.push_back(objectSerialiser.SerialiseToJsonObject(object.at(i)));
        //            //    }
        //            //}
        //            return json;
        //        }
        //    }
        //};

        /// @brief Empty template struct for the serialiser macros to be used to define SerialiserObjects for different types.
        /// @tparam T 
        template<typename T>
        struct DeserialiserObject
        { };

        /// @brief Deserialise a vector.
        /// @tparam T 
        /// @tparam ObjectDesialiser 
        /*template<typename T, bool ObjectDesialiser>
        struct VectorDeserialiser
        {
            std::vector<T> operator()(std::vector<nlohmann::json> const& data)
            {
                using Type = std::remove_pointer_t<std::remove_reference_t<std::remove_all_extents_t<T>>>;
                if constexpr (ObjectDesialiser)
                {
                    DeserialiserObject<Type> objectDeserialiser;
                    std::vector<T> vector;
                    for (auto const& v : data)
                    {
                        if constexpr (std::is_pointer_v<T>)
                        {
                            Type* typePtr = New<Type>();
                            objectDeserialiser.DeserialiseToJsonObject(v, typePtr);
                            vector.push_back(typePtr);
                        }
                        else
                        {
                            Type obj;
                            objectDeserialiser.DeserialiseToJsonObject(v, &obj)
                            vector.push_back(obj);
                        }
                    }
                    return vector;
                }
                else
                {
                    PropertyDeserialiser<Type> propertyDeserialiser;
                    std::vector<T> vector;
                    for (auto const& v : data)
                    {
                        if constexpr (std::is_pointer_v<T>)
                        {
                            Type* typePtr = New<Type>();
                            *typePtr = propertyDeserialiser(v);
                            vector.push_back(typePtr);
                        }
                        else
                        {
                            vector.push_back(propertyDeserialiser(v));
                        }
                    }
                    return vector;
                }
            }
        };*/
    }
}