<?xml version="1.0"?>
<api version="1.0">
	<namespace name="EekXkl">
		<object name="EekXklLayout" parent="EekXkbLayout" type-name="EekXklLayout" get-type="eek_xkl_layout_get_type">
			<method name="disable_option" symbol="eek_xkl_layout_disable_option">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="option" type="gchar*"/>
				</parameters>
			</method>
			<method name="enable_option" symbol="eek_xkl_layout_enable_option">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="option" type="gchar*"/>
				</parameters>
			</method>
			<method name="get_layouts" symbol="eek_xkl_layout_get_layouts">
				<return-type type="gchar**"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
				</parameters>
			</method>
			<method name="get_model" symbol="eek_xkl_layout_get_model">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
				</parameters>
			</method>
			<method name="get_option" symbol="eek_xkl_layout_get_option">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="option" type="gchar*"/>
				</parameters>
			</method>
			<method name="get_options" symbol="eek_xkl_layout_get_options">
				<return-type type="gchar**"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
				</parameters>
			</method>
			<method name="get_variants" symbol="eek_xkl_layout_get_variants">
				<return-type type="gchar**"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_xkl_layout_new">
				<return-type type="EekLayout*"/>
			</constructor>
			<method name="set_config" symbol="eek_xkl_layout_set_config">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="config" type="XklConfigRec*"/>
				</parameters>
			</method>
			<method name="set_config_full" symbol="eek_xkl_layout_set_config_full">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="model" type="gchar*"/>
					<parameter name="layouts" type="gchar**"/>
					<parameter name="variants" type="gchar**"/>
					<parameter name="options" type="gchar**"/>
				</parameters>
			</method>
			<method name="set_layouts" symbol="eek_xkl_layout_set_layouts">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="layouts" type="gchar**"/>
				</parameters>
			</method>
			<method name="set_model" symbol="eek_xkl_layout_set_model">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="model" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_options" symbol="eek_xkl_layout_set_options">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="options" type="gchar**"/>
				</parameters>
			</method>
			<method name="set_variants" symbol="eek_xkl_layout_set_variants">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layout" type="EekXklLayout*"/>
					<parameter name="variants" type="gchar**"/>
				</parameters>
			</method>
			<property name="layouts" type="GStrv*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="model" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="options" type="GStrv*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="variants" type="GStrv*" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<constant name="EEK_XKL_LAYOUT_H" type="int" value="1"/>
	</namespace>
</api>
