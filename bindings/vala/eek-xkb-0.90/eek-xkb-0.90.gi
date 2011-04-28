<?xml version="1.0"?>
<api version="1.0">
	<namespace name="EekXkb">
		<object name="EekXkbLayout" parent="EekLayout" type-name="EekXkbLayout" get-type="eek_xkb_layout_get_type">
			<method name="get_geometry" symbol="eek_xkb_layout_get_geometry">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
				</parameters>
			</method>
			<method name="get_keycodes" symbol="eek_xkb_layout_get_keycodes">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
				</parameters>
			</method>
			<method name="get_symbols" symbol="eek_xkb_layout_get_symbols">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_xkb_layout_new">
				<return-type type="EekLayout*"/>
			</constructor>
			<method name="set_geometry" symbol="eek_xkb_layout_set_geometry">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
					<parameter name="geometry" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_keycodes" symbol="eek_xkb_layout_set_keycodes">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
					<parameter name="keycodes" type="gchar*"/>
				</parameters>
			</method>
<!--
			<method name="set_names" symbol="eek_xkb_layout_set_names">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
					<parameter name="names" type="XkbComponentNamesRec*"/>
				</parameters>
			</method>
-->
			<method name="set_names_full" symbol="eek_xkb_layout_set_names_full">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
				</parameters>
			</method>
			<method name="set_names_full_valist" symbol="eek_xkb_layout_set_names_full_valist">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
					<parameter name="var_args" type="va_list"/>
				</parameters>
			</method>
			<method name="set_symbols" symbol="eek_xkb_layout_set_symbols">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXkbLayout*"/>
					<parameter name="symbols" type="gchar*"/>
				</parameters>
			</method>
			<property name="geometry" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="keycodes" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="symbols" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<constant name="EEK_XKB_LAYOUT_H" type="int" value="1"/>
	</namespace>
</api>
