#pragma once



#define ITERATE_LIST(...) \
    MACRO_EXPAND(ITERATE_LIST_IMPL, __VA_ARGS__)

#define MACRO_EXPAND(macro, ...) \
    macro(__VA_ARGS__)

#define ITERATE_LIST_IMPL(item)	#template item* UFlightComponent::SetupAssociatedComponent(); \