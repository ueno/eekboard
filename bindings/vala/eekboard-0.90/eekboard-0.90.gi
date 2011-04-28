<?xml version="1.0"?>
<api version="1.0">
	<namespace name="Eekboard">
		<object name="EekboardContext" parent="GDBusProxy" type-name="EekboardContext" get-type="eekboard_context_get_type">
			<implements>
				<interface name="GInitable"/>
				<interface name="GAsyncInitable"/>
			</implements>
			<method name="add_keyboard" symbol="eekboard_context_add_keyboard">
				<return-type type="guint"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="keyboard" type="EekKeyboard*"/>
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
			<method name="press_key" symbol="eekboard_context_press_key">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="keycode" type="guint"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="release_key" symbol="eekboard_context_release_key">
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
			<property name="keyboard-visible" type="gboolean" readable="1" writable="0" construct="0" construct-only="0"/>
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
					<parameter name="keycode" type="guint"/>
				</parameters>
			</signal>
			<signal name="key-released" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardContext*"/>
					<parameter name="keycode" type="guint"/>
				</parameters>
			</signal>
		</object>
		<object name="EekboardEekboard" parent="GDBusProxy" type-name="EekboardEekboard" get-type="eekboard_eekboard_get_type">
			<implements>
				<interface name="GInitable"/>
				<interface name="GAsyncInitable"/>
			</implements>
			<method name="create_context" symbol="eekboard_eekboard_create_context">
				<return-type type="EekboardContext*"/>
				<parameters>
					<parameter name="eekboard" type="EekboardEekboard*"/>
					<parameter name="client_name" type="gchar*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="destroy_context" symbol="eekboard_eekboard_destroy_context">
				<return-type type="void"/>
				<parameters>
					<parameter name="eekboard" type="EekboardEekboard*"/>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eekboard_eekboard_new">
				<return-type type="EekboardEekboard*"/>
				<parameters>
					<parameter name="connection" type="GDBusConnection*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</constructor>
			<method name="pop_context" symbol="eekboard_eekboard_pop_context">
				<return-type type="void"/>
				<parameters>
					<parameter name="eekboard" type="EekboardEekboard*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<method name="push_context" symbol="eekboard_eekboard_push_context">
				<return-type type="void"/>
				<parameters>
					<parameter name="eekboard" type="EekboardEekboard*"/>
					<parameter name="context" type="EekboardContext*"/>
					<parameter name="cancellable" type="GCancellable*"/>
				</parameters>
			</method>
			<signal name="destroyed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="EekboardEekboard*"/>
				</parameters>
			</signal>
		</object>
		<constant name="EEKBOARD_CONTEXT_H" type="int" value="1"/>
		<constant name="EEKBOARD_EEKBOARD_H" type="int" value="1"/>
		<constant name="EEKBOARD_H" type="int" value="1"/>
	</namespace>
</api>
