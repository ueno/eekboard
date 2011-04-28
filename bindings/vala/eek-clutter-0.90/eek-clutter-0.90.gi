<?xml version="1.0"?>
<api version="1.0">
	<namespace name="EekClutter">
		<object name="EekClutterKeyboard" parent="ClutterGroup" type-name="EekClutterKeyboard" get-type="eek_clutter_keyboard_get_type">
			<implements>
				<interface name="ClutterScriptable"/>
				<interface name="ClutterAnimatable"/>
				<interface name="AtkImplementor"/>
				<interface name="ClutterContainer"/>
			</implements>
			<constructor name="new" symbol="eek_clutter_keyboard_new">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</constructor>
			<method name="set_theme" symbol="eek_clutter_keyboard_set_theme">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekClutterKeyboard*"/>
					<parameter name="theme" type="EekTheme*"/>
				</parameters>
			</method>
			<property name="keyboard" type="EekKeyboard*" readable="0" writable="1" construct="0" construct-only="1"/>
		</object>
		<constant name="EEK_CLUTTER_KEYBOARD_H" type="int" value="1"/>
	</namespace>
</api>
