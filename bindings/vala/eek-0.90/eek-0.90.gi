<?xml version="1.0"?>
<api version="1.0">
	<namespace name="Eek">
		<callback name="EekCallback">
			<return-type type="void"/>
			<parameters>
				<parameter name="element" type="EekElement*"/>
				<parameter name="user_data" type="gpointer"/>
			</parameters>
		</callback>
		<callback name="EekCompareFunc">
			<return-type type="gint"/>
			<parameters>
				<parameter name="element" type="EekElement*"/>
				<parameter name="user_data" type="gpointer"/>
			</parameters>
		</callback>
		<struct name="EekModifierKey">
			<field name="modifiers" type="EekModifierType"/>
			<field name="key" type="EekKey*"/>
		</struct>
		<struct name="EekThemeClass">
		</struct>
		<struct name="EekThemeContext">
		</struct>
		<struct name="EekThemeNode">
		</struct>
		<boxed name="EekBounds" type-name="EekBounds" get-type="eek_bounds_get_type">
			<method name="copy" symbol="eek_bounds_copy">
				<return-type type="EekBounds*"/>
				<parameters>
					<parameter name="bounds" type="EekBounds*"/>
				</parameters>
			</method>
			<method name="free" symbol="eek_bounds_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="bounds" type="EekBounds*"/>
				</parameters>
			</method>
			<field name="x" type="gdouble"/>
			<field name="y" type="gdouble"/>
			<field name="width" type="gdouble"/>
			<field name="height" type="gdouble"/>
		</boxed>
		<boxed name="EekColor" type-name="EekColor" get-type="eek_color_get_type">
			<method name="copy" symbol="eek_color_copy">
				<return-type type="EekColor*"/>
				<parameters>
					<parameter name="color" type="EekColor*"/>
				</parameters>
			</method>
			<method name="free" symbol="eek_color_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="color" type="EekColor*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_color_new">
				<return-type type="EekColor*"/>
				<parameters>
					<parameter name="red" type="gdouble"/>
					<parameter name="green" type="gdouble"/>
					<parameter name="blue" type="gdouble"/>
					<parameter name="alpha" type="gdouble"/>
				</parameters>
			</constructor>
			<field name="red" type="gdouble"/>
			<field name="green" type="gdouble"/>
			<field name="blue" type="gdouble"/>
			<field name="alpha" type="gdouble"/>
		</boxed>
		<boxed name="EekOutline" type-name="EekOutline" get-type="eek_outline_get_type">
			<method name="copy" symbol="eek_outline_copy">
				<return-type type="EekOutline*"/>
				<parameters>
					<parameter name="outline" type="EekOutline*"/>
				</parameters>
			</method>
			<method name="free" symbol="eek_outline_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="outline" type="EekOutline*"/>
				</parameters>
			</method>
			<field name="corner_radius" type="gdouble"/>
			<field name="points" type="EekPoint*"/>
			<field name="num_points" type="gint"/>
		</boxed>
		<boxed name="EekPoint" type-name="EekPoint" get-type="eek_point_get_type">
			<method name="copy" symbol="eek_point_copy">
				<return-type type="EekPoint*"/>
				<parameters>
					<parameter name="point" type="EekPoint*"/>
				</parameters>
			</method>
			<method name="free" symbol="eek_point_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="point" type="EekPoint*"/>
				</parameters>
			</method>
			<method name="rotate" symbol="eek_point_rotate">
				<return-type type="void"/>
				<parameters>
					<parameter name="point" type="EekPoint*"/>
					<parameter name="angle" type="gint"/>
				</parameters>
			</method>
			<field name="x" type="gdouble"/>
			<field name="y" type="gdouble"/>
		</boxed>
		<boxed name="EekSymbolMatrix" type-name="EekSymbolMatrix" get-type="eek_symbol_matrix_get_type">
			<method name="copy" symbol="eek_symbol_matrix_copy">
				<return-type type="EekSymbolMatrix*"/>
				<parameters>
					<parameter name="matrix" type="EekSymbolMatrix*"/>
				</parameters>
			</method>
			<method name="free" symbol="eek_symbol_matrix_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="matrix" type="EekSymbolMatrix*"/>
				</parameters>
			</method>
			<method name="get_symbol" symbol="eek_symbol_matrix_get_symbol">
				<return-type type="EekSymbol*"/>
				<parameters>
					<parameter name="matrix" type="EekSymbolMatrix*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_symbol_matrix_new">
				<return-type type="EekSymbolMatrix*"/>
				<parameters>
					<parameter name="num_groups" type="gint"/>
					<parameter name="num_levels" type="gint"/>
				</parameters>
			</constructor>
			<method name="set_symbol" symbol="eek_symbol_matrix_set_symbol">
				<return-type type="void"/>
				<parameters>
					<parameter name="matrix" type="EekSymbolMatrix*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
					<parameter name="symbol" type="EekSymbol*"/>
				</parameters>
			</method>
			<field name="num_groups" type="gint"/>
			<field name="num_levels" type="gint"/>
			<field name="data" type="EekSymbol**"/>
		</boxed>
		<enum name="EekGradientType" type-name="EekGradientType" get-type="eek_gradient_type_get_type">
			<member name="EEK_GRADIENT_NONE" value="0"/>
			<member name="EEK_GRADIENT_VERTICAL" value="1"/>
			<member name="EEK_GRADIENT_HORIZONTAL" value="2"/>
			<member name="EEK_GRADIENT_RADIAL" value="3"/>
		</enum>
		<enum name="EekModifierBehavior" type-name="EekModifierBehavior" get-type="eek_modifier_behavior_get_type">
			<member name="EEK_MODIFIER_BEHAVIOR_NONE" value="0"/>
			<member name="EEK_MODIFIER_BEHAVIOR_LOCK" value="1"/>
			<member name="EEK_MODIFIER_BEHAVIOR_LATCH" value="2"/>
		</enum>
		<enum name="EekOrientation" type-name="EekOrientation" get-type="eek_orientation_get_type">
			<member name="EEK_ORIENTATION_VERTICAL" value="0"/>
			<member name="EEK_ORIENTATION_HORIZONTAL" value="1"/>
			<member name="EEK_ORIENTATION_INVALID" value="-1"/>
		</enum>
		<enum name="EekSymbolCategory" type-name="EekSymbolCategory" get-type="eek_symbol_category_get_type">
			<member name="EEK_SYMBOL_CATEGORY_LETTER" value="0"/>
			<member name="EEK_SYMBOL_CATEGORY_FUNCTION" value="1"/>
			<member name="EEK_SYMBOL_CATEGORY_KEYNAME" value="2"/>
			<member name="EEK_SYMBOL_CATEGORY_USER0" value="3"/>
			<member name="EEK_SYMBOL_CATEGORY_USER1" value="4"/>
			<member name="EEK_SYMBOL_CATEGORY_USER2" value="5"/>
			<member name="EEK_SYMBOL_CATEGORY_USER3" value="6"/>
			<member name="EEK_SYMBOL_CATEGORY_USER4" value="7"/>
			<member name="EEK_SYMBOL_CATEGORY_UNKNOWN" value="8"/>
			<member name="EEK_SYMBOL_CATEGORY_LAST" value="8"/>
		</enum>
		<flags name="EekModifierType" type-name="EekModifierType" get-type="eek_modifier_type_get_type">
			<member name="EEK_SHIFT_MASK" value="1"/>
			<member name="EEK_LOCK_MASK" value="2"/>
			<member name="EEK_CONTROL_MASK" value="4"/>
			<member name="EEK_MOD1_MASK" value="8"/>
			<member name="EEK_MOD2_MASK" value="16"/>
			<member name="EEK_MOD3_MASK" value="32"/>
			<member name="EEK_MOD4_MASK" value="64"/>
			<member name="EEK_MOD5_MASK" value="128"/>
			<member name="EEK_BUTTON1_MASK" value="256"/>
			<member name="EEK_BUTTON2_MASK" value="512"/>
			<member name="EEK_BUTTON3_MASK" value="1024"/>
			<member name="EEK_BUTTON4_MASK" value="2048"/>
			<member name="EEK_BUTTON5_MASK" value="4096"/>
			<member name="EEK_SUPER_MASK" value="67108864"/>
			<member name="EEK_HYPER_MASK" value="134217728"/>
			<member name="EEK_META_MASK" value="268435456"/>
			<member name="EEK_RELEASE_MASK" value="1073741824"/>
			<member name="EEK_MODIFIER_MASK" value="1543512063"/>
		</flags>
		<object name="EekContainer" parent="EekElement" type-name="EekContainer" get-type="eek_container_get_type">
			<implements>
				<interface name="EekSerializable"/>
			</implements>
			<method name="add_child" symbol="eek_container_add_child">
				<return-type type="void"/>
				<parameters>
					<parameter name="container" type="EekContainer*"/>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</method>
			<method name="find" symbol="eek_container_find">
				<return-type type="EekElement*"/>
				<parameters>
					<parameter name="container" type="EekContainer*"/>
					<parameter name="func" type="EekCompareFunc"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="foreach_child" symbol="eek_container_foreach_child">
				<return-type type="void"/>
				<parameters>
					<parameter name="container" type="EekContainer*"/>
					<parameter name="callback" type="EekCallback"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<signal name="child-added" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekContainer*"/>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</signal>
			<signal name="child-removed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekContainer*"/>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</signal>
			<vfunc name="add_child">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekContainer*"/>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</vfunc>
			<vfunc name="find">
				<return-type type="EekElement*"/>
				<parameters>
					<parameter name="self" type="EekContainer*"/>
					<parameter name="func" type="EekCompareFunc"/>
					<parameter name="data" type="gpointer"/>
				</parameters>
			</vfunc>
			<vfunc name="foreach_child">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekContainer*"/>
					<parameter name="callback" type="EekCallback"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</vfunc>
			<vfunc name="remove_child">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekContainer*"/>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekElement" parent="GObject" type-name="EekElement" get-type="eek_element_get_type">
			<implements>
				<interface name="EekSerializable"/>
			</implements>
			<method name="get_absolute_position" symbol="eek_element_get_absolute_position">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="x" type="gdouble*"/>
					<parameter name="y" type="gdouble*"/>
				</parameters>
			</method>
			<method name="get_bounds" symbol="eek_element_get_bounds">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="bounds" type="EekBounds*"/>
				</parameters>
			</method>
			<method name="get_group" symbol="eek_element_get_group">
				<return-type type="gint"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</method>
			<method name="get_level" symbol="eek_element_get_level">
				<return-type type="gint"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</method>
			<method name="get_name" symbol="eek_element_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</method>
			<method name="get_parent" symbol="eek_element_get_parent">
				<return-type type="EekElement*"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
				</parameters>
			</method>
			<method name="get_symbol_index" symbol="eek_element_get_symbol_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="group" type="gint*"/>
					<parameter name="level" type="gint*"/>
				</parameters>
			</method>
			<method name="set_bounds" symbol="eek_element_set_bounds">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="bounds" type="EekBounds*"/>
				</parameters>
			</method>
			<method name="set_group" symbol="eek_element_set_group">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="group" type="gint"/>
				</parameters>
			</method>
			<method name="set_level" symbol="eek_element_set_level">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</method>
			<method name="set_name" symbol="eek_element_set_name">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_parent" symbol="eek_element_set_parent">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="parent" type="EekElement*"/>
				</parameters>
			</method>
			<method name="set_position" symbol="eek_element_set_position">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="x" type="gdouble"/>
					<parameter name="y" type="gdouble"/>
				</parameters>
			</method>
			<method name="set_size" symbol="eek_element_set_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="width" type="gdouble"/>
					<parameter name="height" type="gdouble"/>
				</parameters>
			</method>
			<method name="set_symbol_index" symbol="eek_element_set_symbol_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</method>
			<property name="bounds" type="EekBounds*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="group" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="level" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="name" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="symbol-index-changed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</signal>
			<vfunc name="get_bounds">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
					<parameter name="bounds" type="EekBounds*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_parent">
				<return-type type="EekElement*"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_symbol_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
					<parameter name="group" type="gint*"/>
					<parameter name="level" type="gint*"/>
				</parameters>
			</vfunc>
			<vfunc name="set_bounds">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
					<parameter name="bounds" type="EekBounds*"/>
				</parameters>
			</vfunc>
			<vfunc name="set_name">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</vfunc>
			<vfunc name="set_parent">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
					<parameter name="parent" type="EekElement*"/>
				</parameters>
			</vfunc>
			<vfunc name="set_symbol_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekElement*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekKey" parent="EekElement" type-name="EekKey" get-type="eek_key_get_type">
			<implements>
				<interface name="EekSerializable"/>
			</implements>
			<method name="get_index" symbol="eek_key_get_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="column" type="gint*"/>
					<parameter name="row" type="gint*"/>
				</parameters>
			</method>
			<method name="get_keycode" symbol="eek_key_get_keycode">
				<return-type type="guint"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</method>
			<method name="get_oref" symbol="eek_key_get_oref">
				<return-type type="gulong"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</method>
			<method name="get_symbol" symbol="eek_key_get_symbol">
				<return-type type="EekSymbol*"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</method>
			<method name="get_symbol_at_index" symbol="eek_key_get_symbol_at_index">
				<return-type type="EekSymbol*"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
					<parameter name="fallback_group" type="gint"/>
					<parameter name="fallback_level" type="gint"/>
				</parameters>
			</method>
			<method name="get_symbol_matrix" symbol="eek_key_get_symbol_matrix">
				<return-type type="EekSymbolMatrix*"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</method>
			<method name="get_symbol_with_fallback" symbol="eek_key_get_symbol_with_fallback">
				<return-type type="EekSymbol*"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="fallback_group" type="gint"/>
					<parameter name="fallback_level" type="gint"/>
				</parameters>
			</method>
			<method name="is_locked" symbol="eek_key_is_locked">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</method>
			<method name="is_pressed" symbol="eek_key_is_pressed">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</method>
			<method name="set_index" symbol="eek_key_set_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="column" type="gint"/>
					<parameter name="row" type="gint"/>
				</parameters>
			</method>
			<method name="set_keycode" symbol="eek_key_set_keycode">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="keycode" type="guint"/>
				</parameters>
			</method>
			<method name="set_oref" symbol="eek_key_set_oref">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="oref" type="gulong"/>
				</parameters>
			</method>
			<method name="set_symbol_matrix" symbol="eek_key_set_symbol_matrix">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="matrix" type="EekSymbolMatrix*"/>
				</parameters>
			</method>
			<property name="column" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="keycode" type="guint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="oref" type="gulong" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="row" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="symbol-matrix" type="EekSymbolMatrix*" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="cancelled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="locked" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="pressed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="released" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="unlocked" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<vfunc name="get_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="column" type="gint*"/>
					<parameter name="row" type="gint*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_keycode">
				<return-type type="guint"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_oref">
				<return-type type="gulong"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_symbol_matrix">
				<return-type type="EekSymbolMatrix*"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
				</parameters>
			</vfunc>
			<vfunc name="is_locked">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
				</parameters>
			</vfunc>
			<vfunc name="is_pressed">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
				</parameters>
			</vfunc>
			<vfunc name="set_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="column" type="gint"/>
					<parameter name="row" type="gint"/>
				</parameters>
			</vfunc>
			<vfunc name="set_keycode">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="keycode" type="guint"/>
				</parameters>
			</vfunc>
			<vfunc name="set_oref">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="oref" type="gulong"/>
				</parameters>
			</vfunc>
			<vfunc name="set_symbol_matrix">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="matrix" type="EekSymbolMatrix*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekKeyboard" parent="EekContainer" type-name="EekKeyboard" get-type="eek_keyboard_get_type">
			<implements>
				<interface name="EekSerializable"/>
			</implements>
			<method name="add_outline" symbol="eek_keyboard_add_outline">
				<return-type type="gulong"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="outline" type="EekOutline*"/>
				</parameters>
			</method>
			<method name="create_section" symbol="eek_keyboard_create_section">
				<return-type type="EekSection*"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="find_key_by_keycode" symbol="eek_keyboard_find_key_by_keycode">
				<return-type type="EekKey*"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="keycode" type="guint"/>
				</parameters>
			</method>
			<method name="get_alt_gr_mask" symbol="eek_keyboard_get_alt_gr_mask">
				<return-type type="EekModifierType"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_group" symbol="eek_keyboard_get_group">
				<return-type type="gint"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_layout" symbol="eek_keyboard_get_layout">
				<return-type type="EekLayout*"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_level" symbol="eek_keyboard_get_level">
				<return-type type="gint"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_locked_keys" symbol="eek_keyboard_get_locked_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_modifier_behavior" symbol="eek_keyboard_get_modifier_behavior">
				<return-type type="EekModifierBehavior"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_modifiers" symbol="eek_keyboard_get_modifiers">
				<return-type type="EekModifierType"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_num_lock_mask" symbol="eek_keyboard_get_num_lock_mask">
				<return-type type="EekModifierType"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_outline" symbol="eek_keyboard_get_outline">
				<return-type type="EekOutline*"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="oref" type="gulong"/>
				</parameters>
			</method>
			<method name="get_pressed_keys" symbol="eek_keyboard_get_pressed_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_size" symbol="eek_keyboard_get_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="width" type="gdouble*"/>
					<parameter name="height" type="gdouble*"/>
				</parameters>
			</method>
			<method name="get_symbol_index" symbol="eek_keyboard_get_symbol_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="group" type="gint*"/>
					<parameter name="level" type="gint*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_keyboard_new">
				<return-type type="EekKeyboard*"/>
				<parameters>
					<parameter name="layout" type="EekLayout*"/>
					<parameter name="initial_width" type="gdouble"/>
					<parameter name="initial_height" type="gdouble"/>
				</parameters>
			</constructor>
			<method name="output" symbol="eek_keyboard_output">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="output" type="GString*"/>
					<parameter name="indent" type="gint"/>
				</parameters>
			</method>
			<method name="set_alt_gr_mask" symbol="eek_keyboard_set_alt_gr_mask">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="alt_gr_mask" type="EekModifierType"/>
				</parameters>
			</method>
			<method name="set_group" symbol="eek_keyboard_set_group">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="group" type="gint"/>
				</parameters>
			</method>
			<method name="set_level" symbol="eek_keyboard_set_level">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</method>
			<method name="set_modifier_behavior" symbol="eek_keyboard_set_modifier_behavior">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="modifier_behavior" type="EekModifierBehavior"/>
				</parameters>
			</method>
			<method name="set_modifiers" symbol="eek_keyboard_set_modifiers">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="modifiers" type="EekModifierType"/>
				</parameters>
			</method>
			<method name="set_num_lock_mask" symbol="eek_keyboard_set_num_lock_mask">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="num_lock_mask" type="EekModifierType"/>
				</parameters>
			</method>
			<method name="set_size" symbol="eek_keyboard_set_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="width" type="gdouble"/>
					<parameter name="height" type="gdouble"/>
				</parameters>
			</method>
			<method name="set_symbol_index" symbol="eek_keyboard_set_symbol_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</method>
			<property name="layout" type="EekLayout*" readable="1" writable="1" construct="0" construct-only="1"/>
			<property name="modifier-behavior" type="EekModifierBehavior" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="key-cancelled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-locked" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-pressed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-released" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-unlocked" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<vfunc name="create_section">
				<return-type type="EekSection*"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
				</parameters>
			</vfunc>
			<vfunc name="find_key_by_keycode">
				<return-type type="EekKey*"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="keycode" type="guint"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekKeysym" parent="EekSymbol" type-name="EekKeysym" get-type="eek_keysym_get_type">
			<implements>
				<interface name="EekSerializable"/>
			</implements>
			<method name="get_xkeysym" symbol="eek_keysym_get_xkeysym">
				<return-type type="guint"/>
				<parameters>
					<parameter name="keysym" type="EekKeysym*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_keysym_new">
				<return-type type="EekKeysym*"/>
				<parameters>
					<parameter name="xkeysym" type="guint"/>
				</parameters>
			</constructor>
			<constructor name="new_from_name" symbol="eek_keysym_new_from_name">
				<return-type type="EekKeysym*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</constructor>
			<constructor name="new_with_modifier" symbol="eek_keysym_new_with_modifier">
				<return-type type="EekKeysym*"/>
				<parameters>
					<parameter name="xkeysym" type="guint"/>
					<parameter name="modifier" type="EekModifierType"/>
				</parameters>
			</constructor>
		</object>
		<object name="EekLayout" parent="GObject" type-name="EekLayout" get-type="eek_layout_get_type">
			<vfunc name="create_keyboard">
				<return-type type="EekKeyboard*"/>
				<parameters>
					<parameter name="self" type="EekLayout*"/>
					<parameter name="initial_width" type="gdouble"/>
					<parameter name="initial_height" type="gdouble"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekSection" parent="EekContainer" type-name="EekSection" get-type="eek_section_get_type">
			<implements>
				<interface name="EekSerializable"/>
			</implements>
			<method name="add_row" symbol="eek_section_add_row">
				<return-type type="void"/>
				<parameters>
					<parameter name="section" type="EekSection*"/>
					<parameter name="num_columns" type="gint"/>
					<parameter name="orientation" type="EekOrientation"/>
				</parameters>
			</method>
			<method name="create_key" symbol="eek_section_create_key">
				<return-type type="EekKey*"/>
				<parameters>
					<parameter name="section" type="EekSection*"/>
					<parameter name="column" type="gint"/>
					<parameter name="row" type="gint"/>
				</parameters>
			</method>
			<method name="find_key_by_keycode" symbol="eek_section_find_key_by_keycode">
				<return-type type="EekKey*"/>
				<parameters>
					<parameter name="section" type="EekSection*"/>
					<parameter name="keycode" type="guint"/>
				</parameters>
			</method>
			<method name="get_angle" symbol="eek_section_get_angle">
				<return-type type="gint"/>
				<parameters>
					<parameter name="section" type="EekSection*"/>
				</parameters>
			</method>
			<method name="get_n_rows" symbol="eek_section_get_n_rows">
				<return-type type="gint"/>
				<parameters>
					<parameter name="section" type="EekSection*"/>
				</parameters>
			</method>
			<method name="get_row" symbol="eek_section_get_row">
				<return-type type="void"/>
				<parameters>
					<parameter name="section" type="EekSection*"/>
					<parameter name="index" type="gint"/>
					<parameter name="num_columns" type="gint*"/>
					<parameter name="orientation" type="EekOrientation*"/>
				</parameters>
			</method>
			<method name="set_angle" symbol="eek_section_set_angle">
				<return-type type="void"/>
				<parameters>
					<parameter name="section" type="EekSection*"/>
					<parameter name="angle" type="gint"/>
				</parameters>
			</method>
			<property name="angle" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="key-cancelled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-locked" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-pressed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-released" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-unlocked" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<vfunc name="add_row">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="num_columns" type="gint"/>
					<parameter name="orientation" type="EekOrientation"/>
				</parameters>
			</vfunc>
			<vfunc name="create_key">
				<return-type type="EekKey*"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="row" type="gint"/>
					<parameter name="column" type="gint"/>
				</parameters>
			</vfunc>
			<vfunc name="find_key_by_keycode">
				<return-type type="EekKey*"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="keycode" type="guint"/>
				</parameters>
			</vfunc>
			<vfunc name="get_angle">
				<return-type type="gint"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_n_rows">
				<return-type type="gint"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_row">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="index" type="gint"/>
					<parameter name="num_columns" type="gint*"/>
					<parameter name="orientation" type="EekOrientation*"/>
				</parameters>
			</vfunc>
			<vfunc name="set_angle">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekSection*"/>
					<parameter name="angle" type="gint"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekSymbol" parent="GObject" type-name="EekSymbol" get-type="eek_symbol_get_type">
			<implements>
				<interface name="EekSerializable"/>
			</implements>
			<method name="get_category" symbol="eek_symbol_get_category">
				<return-type type="EekSymbolCategory"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
				</parameters>
			</method>
			<method name="get_icon_name" symbol="eek_symbol_get_icon_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
				</parameters>
			</method>
			<method name="get_label" symbol="eek_symbol_get_label">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
				</parameters>
			</method>
			<method name="get_modifier_mask" symbol="eek_symbol_get_modifier_mask">
				<return-type type="EekModifierType"/>
				<parameters>
					<parameter name="keysym" type="EekSymbol*"/>
				</parameters>
			</method>
			<method name="get_name" symbol="eek_symbol_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
				</parameters>
			</method>
			<method name="is_modifier" symbol="eek_symbol_is_modifier">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_symbol_new">
				<return-type type="EekSymbol*"/>
				<parameters>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</constructor>
			<method name="set_category" symbol="eek_symbol_set_category">
				<return-type type="void"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
					<parameter name="category" type="EekSymbolCategory"/>
				</parameters>
			</method>
			<method name="set_icon_name" symbol="eek_symbol_set_icon_name">
				<return-type type="void"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
					<parameter name="icon_name" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_label" symbol="eek_symbol_set_label">
				<return-type type="void"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
					<parameter name="label" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_modifier_mask" symbol="eek_symbol_set_modifier_mask">
				<return-type type="void"/>
				<parameters>
					<parameter name="keysym" type="EekSymbol*"/>
					<parameter name="mask" type="EekModifierType"/>
				</parameters>
			</method>
			<method name="set_name" symbol="eek_symbol_set_name">
				<return-type type="void"/>
				<parameters>
					<parameter name="symbol" type="EekSymbol*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<property name="category" type="EekSymbolCategory" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="icon-name" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="label" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="modifier-mask" type="EekModifierType" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="name" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
		</object>
		<object name="EekTheme" parent="GObject" type-name="EekTheme" get-type="eek_theme_get_type">
			<method name="load_stylesheet" symbol="eek_theme_load_stylesheet">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="theme" type="EekTheme*"/>
					<parameter name="path" type="char*"/>
					<parameter name="error" type="GError**"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_theme_new">
				<return-type type="EekTheme*"/>
				<parameters>
					<parameter name="application_stylesheet" type="char*"/>
					<parameter name="theme_stylesheet" type="char*"/>
					<parameter name="default_stylesheet" type="char*"/>
				</parameters>
			</constructor>
			<method name="unload_stylesheet" symbol="eek_theme_unload_stylesheet">
				<return-type type="void"/>
				<parameters>
					<parameter name="theme" type="EekTheme*"/>
					<parameter name="path" type="char*"/>
				</parameters>
			</method>
			<property name="application-stylesheet" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
			<property name="default-stylesheet" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
			<property name="theme-stylesheet" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
		</object>
		<object name="EekXmlLayout" parent="EekLayout" type-name="EekXmlLayout" get-type="eek_xml_layout_get_type">
			<method name="get_source" symbol="eek_xml_layout_get_source">
				<return-type type="GInputStream*"/>
				<parameters>
					<parameter name="layout" type="EekXmlLayout*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_xml_layout_new">
				<return-type type="EekLayout*"/>
				<parameters>
					<parameter name="source" type="GInputStream*"/>
				</parameters>
			</constructor>
			<method name="set_source" symbol="eek_xml_layout_set_source">
				<return-type type="void"/>
				<parameters>
					<parameter name="layout" type="EekXmlLayout*"/>
					<parameter name="source" type="GInputStream*"/>
				</parameters>
			</method>
			<property name="source" type="GInputStream*" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<interface name="EekSerializable" type-name="EekSerializable" get-type="eek_serializable_get_type">
			<method name="deserialize" symbol="eek_serializable_deserialize">
				<return-type type="EekSerializable*"/>
				<parameters>
					<parameter name="variant" type="GVariant*"/>
				</parameters>
			</method>
			<method name="serialize" symbol="eek_serializable_serialize">
				<return-type type="GVariant*"/>
				<parameters>
					<parameter name="object" type="EekSerializable*"/>
				</parameters>
			</method>
			<vfunc name="deserialize">
				<return-type type="gsize"/>
				<parameters>
					<parameter name="object" type="EekSerializable*"/>
					<parameter name="variant" type="GVariant*"/>
					<parameter name="index" type="gsize"/>
				</parameters>
			</vfunc>
			<vfunc name="serialize">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="EekSerializable*"/>
					<parameter name="builder" type="GVariantBuilder*"/>
				</parameters>
			</vfunc>
		</interface>
		<constant name="EEK_CONTAINER_H" type="int" value="1"/>
		<constant name="EEK_ELEMENT_H" type="int" value="1"/>
		<constant name="EEK_INVALID_KEYCODE" type="int" value="0"/>
		<constant name="EEK_INVALID_KEYSYM" type="int" value="0"/>
		<constant name="EEK_KEYBOARD_H" type="int" value="1"/>
		<constant name="EEK_KEYSYM_H" type="int" value="1"/>
		<constant name="EEK_KEY_H" type="int" value="1"/>
		<constant name="EEK_LAYOUT_H" type="int" value="1"/>
		<constant name="EEK_SECTION_H" type="int" value="1"/>
		<constant name="EEK_SERIALIZABLE_H" type="int" value="1"/>
		<constant name="EEK_SYMBOL_H" type="int" value="1"/>
		<constant name="EEK_SYMBOL_MATRIX_H" type="int" value="1"/>
		<constant name="EEK_TYPES_H" type="int" value="1"/>
		<constant name="EEK_XML_H" type="int" value="1"/>
		<constant name="EEK_XML_LAYOUT_H" type="int" value="1"/>
		<constant name="EEK_XML_SCHEMA_VERSION" type="char*" value="0.90"/>
	</namespace>
</api>
