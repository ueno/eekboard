<?xml version="1.0"?>
<api version="1.0">
	<namespace name="Eek">
		<function name="keysym_get_category" symbol="eek_keysym_get_category">
			<return-type type="EekKeysymCategory"/>
			<parameters>
				<parameter name="keysym" type="guint"/>
			</parameters>
		</function>
		<function name="keysym_to_string" symbol="eek_keysym_to_string">
			<return-type type="gchar*"/>
			<parameters>
				<parameter name="keysym" type="guint"/>
			</parameters>
		</function>
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
		<boxed name="EekBounds" type-name="EekBounds" get-type="eek_bounds_get_type">
			<field name="x" type="gdouble"/>
			<field name="y" type="gdouble"/>
			<field name="width" type="gdouble"/>
			<field name="height" type="gdouble"/>
		</boxed>
		<boxed name="EekKeysymMatrix" type-name="EekKeysymMatrix" get-type="eek_keysym_matrix_get_type">
			<field name="data" type="guint*"/>
			<field name="num_groups" type="gint"/>
			<field name="num_levels" type="gint"/>
		</boxed>
		<boxed name="EekOutline" type-name="EekOutline" get-type="eek_outline_get_type">
			<field name="corner_radius" type="gdouble"/>
			<field name="points" type="EekPoint*"/>
			<field name="num_points" type="gint"/>
		</boxed>
		<boxed name="EekPoint" type-name="EekPoint" get-type="eek_point_get_type">
			<field name="x" type="gdouble"/>
			<field name="y" type="gdouble"/>
		</boxed>
		<enum name="EekKeysymCategory">
			<member name="EEK_KEYSYM_CATEGORY_LETTER" value="0"/>
			<member name="EEK_KEYSYM_CATEGORY_FUNCTION" value="1"/>
			<member name="EEK_KEYSYM_CATEGORY_KEYNAME" value="2"/>
			<member name="EEK_KEYSYM_CATEGORY_UNKNOWN" value="3"/>
			<member name="EEK_KEYSYM_CATEGORY_LAST" value="3"/>
		</enum>
		<enum name="EekOrientation">
			<member name="EEK_ORIENTATION_VERTICAL" value="0"/>
			<member name="EEK_ORIENTATION_HORIZONTAL" value="1"/>
			<member name="EEK_ORIENTATION_INVALID" value="-1"/>
		</enum>
		<object name="EekContainer" parent="EekElement" type-name="EekContainer" get-type="eek_container_get_type">
			<method name="find" symbol="eek_container_find">
				<return-type type="EekElement*"/>
				<parameters>
					<parameter name="container" type="EekContainer*"/>
					<parameter name="func" type="EekCompareFunc"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="find_by_position" symbol="eek_container_find_by_position">
				<return-type type="EekElement*"/>
				<parameters>
					<parameter name="container" type="EekContainer*"/>
					<parameter name="x" type="gdouble"/>
					<parameter name="y" type="gdouble"/>
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
					<parameter name="user_data" type="gpointer"/>
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
		<object name="EekElement" parent="GInitiallyUnowned" type-name="EekElement" get-type="eek_element_get_type">
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
			<method name="set_bounds" symbol="eek_element_set_bounds">
				<return-type type="void"/>
				<parameters>
					<parameter name="element" type="EekElement*"/>
					<parameter name="bounds" type="EekBounds*"/>
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
			<property name="bounds" type="EekBounds*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="name" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
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
		</object>
		<object name="EekKey" parent="EekElement" type-name="EekKey" get-type="eek_key_get_type">
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
			<method name="get_keysym" symbol="eek_key_get_keysym">
				<return-type type="guint"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</method>
			<method name="get_keysym_index" symbol="eek_key_get_keysym_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="group" type="gint*"/>
					<parameter name="level" type="gint*"/>
				</parameters>
			</method>
			<method name="get_keysyms" symbol="eek_key_get_keysyms">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="keysyms" type="guint**"/>
					<parameter name="num_groups" type="gint*"/>
					<parameter name="num_levels" type="gint*"/>
				</parameters>
			</method>
			<method name="get_outline" symbol="eek_key_get_outline">
				<return-type type="EekOutline*"/>
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
			<method name="set_keysym_index" symbol="eek_key_set_keysym_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</method>
			<method name="set_keysyms" symbol="eek_key_set_keysyms">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="keysyms" type="guint*"/>
					<parameter name="num_groups" type="gint"/>
					<parameter name="num_levels" type="gint"/>
				</parameters>
			</method>
			<method name="set_outline" symbol="eek_key_set_outline">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
					<parameter name="outline" type="EekOutline*"/>
				</parameters>
			</method>
			<property name="column" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="group" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="keycode" type="guint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="keysyms" type="EekKeysymMatrix*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="level" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="outline" type="gpointer" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="row" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="pressed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="key" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="released" when="FIRST">
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
			<vfunc name="get_keysym">
				<return-type type="guint"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_keysym_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="group" type="gint*"/>
					<parameter name="level" type="gint*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_keysyms">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="keysyms" type="guint**"/>
					<parameter name="num_groups" type="gint*"/>
					<parameter name="num_levels" type="gint*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_outline">
				<return-type type="EekOutline*"/>
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
			<vfunc name="set_keysym_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</vfunc>
			<vfunc name="set_keysyms">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="keysyms" type="guint*"/>
					<parameter name="num_groups" type="gint"/>
					<parameter name="num_levels" type="gint"/>
				</parameters>
			</vfunc>
			<vfunc name="set_outline">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKey*"/>
					<parameter name="outline" type="EekOutline*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekKeyboard" parent="EekContainer" type-name="EekKeyboard" get-type="eek_keyboard_get_type">
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
			<method name="get_keysym_index" symbol="eek_keyboard_get_keysym_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="group" type="gint*"/>
					<parameter name="level" type="gint*"/>
				</parameters>
			</method>
			<method name="realize" symbol="eek_keyboard_realize">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="set_keysym_index" symbol="eek_keyboard_set_keysym_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</method>
			<method name="set_layout" symbol="eek_keyboard_set_layout">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
					<parameter name="layout" type="EekLayout*"/>
				</parameters>
			</method>
			<property name="group" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="level" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="key-pressed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="EekKeyboard*"/>
					<parameter name="p0" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-released" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="EekKeyboard*"/>
					<parameter name="p0" type="EekKey*"/>
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
			<vfunc name="get_keysym_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="group" type="gint*"/>
					<parameter name="level" type="gint*"/>
				</parameters>
			</vfunc>
			<vfunc name="realize">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
				</parameters>
			</vfunc>
			<vfunc name="set_keysym_index">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="group" type="gint"/>
					<parameter name="level" type="gint"/>
				</parameters>
			</vfunc>
			<vfunc name="set_layout">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekKeyboard*"/>
					<parameter name="layout" type="EekLayout*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekSection" parent="EekContainer" type-name="EekSection" get-type="eek_section_get_type">
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
			<signal name="key-pressed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="EekSection*"/>
					<parameter name="p0" type="EekKey*"/>
				</parameters>
			</signal>
			<signal name="key-released" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="EekSection*"/>
					<parameter name="p0" type="EekKey*"/>
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
		<interface name="EekLayout" type-name="EekLayout" get-type="eek_layout_get_type">
			<method name="apply" symbol="eek_layout_apply">
				<return-type type="void"/>
				<parameters>
					<parameter name="layout" type="EekLayout*"/>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</method>
			<method name="get_group" symbol="eek_layout_get_group">
				<return-type type="gint"/>
				<parameters>
					<parameter name="layout" type="EekLayout*"/>
				</parameters>
			</method>
			<signal name="changed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekLayout*"/>
				</parameters>
			</signal>
			<signal name="group-changed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekLayout*"/>
					<parameter name="group" type="gint"/>
				</parameters>
			</signal>
			<vfunc name="apply">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekLayout*"/>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_group">
				<return-type type="gint"/>
				<parameters>
					<parameter name="self" type="EekLayout*"/>
				</parameters>
			</vfunc>
		</interface>
		<constant name="EEK_CONTAINER_H" type="int" value="1"/>
		<constant name="EEK_ELEMENT_H" type="int" value="1"/>
		<constant name="EEK_KEYBOARD_H" type="int" value="1"/>
		<constant name="EEK_KEYSYM_H" type="int" value="1"/>
		<constant name="EEK_KEY_H" type="int" value="1"/>
		<constant name="EEK_LAYOUT_H" type="int" value="1"/>
		<constant name="EEK_SECTION_H" type="int" value="1"/>
		<constant name="EEK_TYPES_H" type="int" value="1"/>
	</namespace>
</api>
