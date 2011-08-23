<?xml version="1.0"?>
<api version="1.0">
	<namespace name="Eekboard">
		<function name="xkl_config_rec_from_string" symbol="eekboard_xkl_config_rec_from_string">
			<return-type type="XklConfigRec*"/>
			<parameters>
				<parameter name="layouts" type="gchar*"/>
			</parameters>
		</function>
		<function name="xkl_config_rec_to_string" symbol="eekboard_xkl_config_rec_to_string">
			<return-type type="gchar*"/>
			<parameters>
				<parameter name="rec" type="XklConfigRec*"/>
			</parameters>
		</function>
		<function name="xkl_list_layout_variants" symbol="eekboard_xkl_list_layout_variants">
			<return-type type="GSList*"/>
			<parameters>
				<parameter name="registry" type="XklConfigRegistry*"/>
				<parameter name="layout" type="gchar*"/>
			</parameters>
		</function>
		<function name="xkl_list_layouts" symbol="eekboard_xkl_list_layouts">
			<return-type type="GSList*"/>
			<parameters>
				<parameter name="registry" type="XklConfigRegistry*"/>
			</parameters>
		</function>
		<function name="xkl_list_models" symbol="eekboard_xkl_list_models">
			<return-type type="GSList*"/>
			<parameters>
				<parameter name="registry" type="XklConfigRegistry*"/>
			</parameters>
		</function>
		<function name="xkl_list_option_groups" symbol="eekboard_xkl_list_option_groups">
			<return-type type="GSList*"/>
			<parameters>
				<parameter name="registry" type="XklConfigRegistry*"/>
			</parameters>
		</function>
		<function name="xkl_list_options" symbol="eekboard_xkl_list_options">
			<return-type type="GSList*"/>
			<parameters>
				<parameter name="registry" type="XklConfigRegistry*"/>
				<parameter name="group" type="gchar*"/>
			</parameters>
		</function>
		<object name="EekboardClient" parent="GDBusProxy" type-name="EekboardClient" get-type="eekboard_client_get_type">
			<implements>
				<interface name="GInitable"/>
				<interface name="GAsyncInitable"/>
			</implements>
			<method name="create_context" symbol="eekboard_client_create_context">
				<return-type type="EekboardContext*"/>
				<parameters>
					<parameter name="eekboard" type="EekboardClient*"/>
					<parameter name="client_name" type="gchar*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="destroy_context" symbol="eekboard_client_destroy_context">
				<return-type type="void"/>
				<parameters>
					<parameter name="eekboard" type="EekboardClient*"/>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eekboard_client_new">
				<return-type type="EekboardClient*"/>
				<parameters>
					<parameter name="connection" type="GDBusConnection*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</constructor>
			<method name="pop_context" symbol="eekboard_client_pop_context">
				<return-type type="void"/>
				<parameters>
					<parameter name="eekboard" type="EekboardClient*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="push_context" symbol="eekboard_client_push_context">
				<return-type type="void"/>
				<parameters>
					<parameter name="eekboard" type="EekboardClient*"/>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<signal name="destroyed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardClient*"/>
				</parameters>
			</signal>
		</object>
		<object name="EekboardContext" parent="GDBusProxy" type-name="EekboardContext" get-type="eekboard_context_get_type">
			<implements>
				<interface name="GInitable"/>
				<interface name="GAsyncInitable"/>
			</implements>
			<method name="add_keyboard" symbol="eekboard_context_add_keyboard">
				<return-type type="guint"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="keyboard" type="gchar*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="get_group" symbol="eekboard_context_get_group">
				<return-type type="gint"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="hide_keyboard" symbol="eekboard_context_hide_keyboard">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="is_enabled" symbol="eekboard_context_is_enabled">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
				</parameters>
			</method>
			<method name="is_keyboard_visible" symbol="eekboard_context_is_keyboard_visible">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eekboard_context_new">
				<return-type type="EekboardContext*"/>
				<parameters>
					<parameter name="connection" type="GDBusConnection*"/>
					<parameter name="object_path" type="gchar*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</constructor>
			<method name="press_keycode" symbol="eekboard_context_press_keycode">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="keycode" type="guint"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="release_keycode" symbol="eekboard_context_release_keycode">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="keycode" type="guint"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="remove_keyboard" symbol="eekboard_context_remove_keyboard">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="keyboard_id" type="guint"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="set_enabled" symbol="eekboard_context_set_enabled">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="enabled" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_fullscreen" symbol="eekboard_context_set_fullscreen">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="fullscreen" type="gboolean"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="set_group" symbol="eekboard_context_set_group">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="group" type="gint"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="set_keyboard" symbol="eekboard_context_set_keyboard">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="keyboard_id" type="guint"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="show_keyboard" symbol="eekboard_context_show_keyboard">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<property name="visible" type="gboolean" readable="1" writable="0" construct="0" construct-only="0"/>
			<signal name="destroyed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContext*"/>
				</parameters>
			</signal>
			<signal name="disabled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContext*"/>
				</parameters>
			</signal>
			<signal name="enabled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContext*"/>
				</parameters>
			</signal>
			<signal name="key-pressed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContext*"/>
					<parameter name="keyname" type="char*"/>
					<parameter name="symbol" type="GObject*"/>
					<parameter name="modifiers" type="guint"/>
				</parameters>
			</signal>
		</object>
		<object name="EekboardContextService" parent="GObject" type-name="EekboardContextService" get-type="eekboard_context_service_get_type">
			<method name="disable" symbol="eekboard_context_service_disable">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContextService*"/>
				</parameters>
			</method>
			<method name="enable" symbol="eekboard_context_service_enable">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContextService*"/>
				</parameters>
			</method>
			<method name="get_client_name" symbol="eekboard_context_service_get_client_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="context" type="EekboardContextService*"/>
				</parameters>
			</method>
			<method name="get_fullscreen" symbol="eekboard_context_service_get_fullscreen">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="context" type="EekboardContextService*"/>
				</parameters>
			</method>
			<method name="get_keyboard" symbol="eekboard_context_service_get_keyboard">
				<return-type type="EekKeyboard*"/>
				<parameters>
					<parameter name="context" type="EekboardContextService*"/>
				</parameters>
			</method>
			<property name="client-name" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="connection" type="GDBusConnection*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="fullscreen" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="keyboard" type="EekKeyboard*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="object-path" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="visible" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="disabled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContextService*"/>
				</parameters>
			</signal>
			<signal name="enabled" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContextService*"/>
				</parameters>
			</signal>
			<vfunc name="create_keyboard">
				<return-type type="EekKeyboard*"/>
				<parameters>
					<parameter name="self" type="EekboardContextService*"/>
					<parameter name="keyboard_type" type="gchar*"/>
				</parameters>
			</vfunc>
			<vfunc name="hide_keyboard">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContextService*"/>
				</parameters>
			</vfunc>
			<vfunc name="show_keyboard">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContextService*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="EekboardService" parent="GObject" type-name="EekboardService" get-type="eekboard_service_get_type">
			<constructor name="new" symbol="eekboard_service_new">
				<return-type type="EekboardService*"/>
				<parameters>
					<parameter name="object_path" type="gchar*"/>
					<parameter name="connection" type="GDBusConnection*"/>
				</parameters>
			</constructor>
			<property name="connection" type="GDBusConnection*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="object-path" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<signal name="destroyed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="EekboardService*"/>
				</parameters>
			</signal>
			<vfunc name="create_context">
				<return-type type="EekboardContextService*"/>
				<parameters>
					<parameter name="self" type="EekboardService*"/>
					<parameter name="client_name" type="gchar*"/>
					<parameter name="object_path" type="gchar*"/>
				</parameters>
			</vfunc>
		</object>
		<constant name="EEKBOARD_CLIENT_H" type="int" value="1"/>
		<constant name="EEKBOARD_CONTEXT_H" type="int" value="1"/>
		<constant name="EEKBOARD_CONTEXT_SERVICE_H" type="int" value="1"/>
		<constant name="EEKBOARD_CONTEXT_SERVICE_INTERFACE" type="char*" value="org.fedorahosted.Eekboard.Context"/>
		<constant name="EEKBOARD_CONTEXT_SERVICE_PATH" type="char*" value="/org/fedorahosted/Eekboard/Context_%d"/>
		<constant name="EEKBOARD_SERVICE_H" type="int" value="1"/>
		<constant name="EEKBOARD_SERVICE_INTERFACE" type="char*" value="org.fedorahosted.Eekboard"/>
		<constant name="EEKBOARD_SERVICE_PATH" type="char*" value="/org/fedorahosted/Eekboard"/>
		<constant name="EEKBOARD_XKLUTIL_H" type="int" value="1"/>
	</namespace>
</api>
