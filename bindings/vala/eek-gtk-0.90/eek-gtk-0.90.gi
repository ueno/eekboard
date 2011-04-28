<?xml version="1.0"?>
<api version="1.0">
	<namespace name="EekGtk">
		<object name="EekGtkKeyboard" parent="GtkDrawingArea" type-name="EekGtkKeyboard" get-type="eek_gtk_keyboard_get_type">
			<implements>
				<interface name="AtkImplementor"/>
				<interface name="GtkBuildable"/>
			</implements>
			<constructor name="new" symbol="eek_gtk_keyboard_new">
				<return-type type="GtkWidget*"/>
				<parameters>
					<parameter name="keyboard" type="EekKeyboard*"/>
				</parameters>
			</constructor>
			<method name="set_theme" symbol="eek_gtk_keyboard_set_theme">
				<return-type type="void"/>
				<parameters>
					<parameter name="keyboard" type="EekGtkKeyboard*"/>
					<parameter name="theme" type="EekTheme*"/>
				</parameters>
			</method>
			<property name="keyboard" type="EekKeyboard*" readable="0" writable="1" construct="0" construct-only="1"/>
		</object>
		<constant name="EEK_GTK_KEYBOARD_H" type="int" value="1"/>
	</namespace>
</api>
