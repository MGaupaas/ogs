/**
 * \file
 * \author Norbert Grunwald
 * \date   Sep 7, 2017
 *
 * \copyright
 * Copyright (c) 2012-2019, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "CreateComponent.h"

#include "BaseLib/ConfigTree.h"

#include "Components/Components.h"
#include "CreateProperty.h"

namespace
{
std::unique_ptr<MaterialPropertyLib::Component> createComponent(
    BaseLib::ConfigTree const& config)
{
    using namespace MaterialPropertyLib;
    // Parsing the component name
    //! \ogs_file_param{prj__media__medium__phases__phase__components__component__name}
    auto const& component_name = config.getConfigParameter<std::string>("name");

    // Check whether a name is given or not
    if (component_name.empty())
    {
        OGS_FATAL("Component name is a mandatory field and cannot be empty.");
    }

    // Parsing component properties. If a component name is given and this
    // component is predefined in the class implementation, properties
    // become optional. The default values of properties will be overwritten
    // if specified.
    std::unique_ptr<PropertyArray> properties =
        //! \ogs_file_param{prj__media__medium__phases__phase__components__component__properties}
        createProperties(config.getConfigSubtreeOptional("properties"));

    // If a name is given, it must conform with one of the derived component
    // names in the following list:
    if (boost::iequals(component_name, "water"))
    {
        return std::make_unique<Water>(std::move(properties));
    }

    if (!properties)
    {
        // No component properties are provided. If the component is not
        // specified, this results in a fatal error, since an unspecified
        // component has no properties.
        OGS_FATAL("No Properties defined for unspecified component");
    }

    return std::make_unique<Component>(component_name, std::move(properties));
}
}

namespace MaterialPropertyLib
{
std::vector<std::unique_ptr<Component>> createComponents(
    boost::optional<BaseLib::ConfigTree> const& config)
{
    if (!config)
    {
        return {};
    }

    std::vector<std::unique_ptr<Component>> components;

    // Collect component's names to avoid duplicate components.
    std::set<std::string> component_names;

    for (
        auto const& component_config :
        //! \ogs_file_param{prj__media__medium__phases__phase__components__component}
        config->getConfigSubtreeList("component"))
    {
        auto component = createComponent(component_config);

        bool new_insertion = false;
        std::tie(std::ignore, new_insertion) =
            component_names.insert(component->name());
        if (!new_insertion)
        {
            OGS_FATAL("Found duplicates with the same component name tag '%s'.",
                      component->name().c_str());
        }

        components.push_back(std::move(component));
    }
    return components;
}
}  // namespace MaterialPropertyLib
