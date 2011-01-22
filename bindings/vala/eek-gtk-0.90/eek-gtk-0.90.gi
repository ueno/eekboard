<?xml version="1.0"?>
<api version="1.0">
	<namespace name="EekGtk">
		<object name="EekGtkKeyboard" parent="EekKeyboard" type-name="EekGtkKeyboard" get-type="eek_gtk_keyboard_get_type">
			<method name="get_widget" symbol="eek_gtk_keyboard_get_widget">
				<return-type type="GtkWidget*"/>
				<parameters>
					<parameter name="keyboard" type="EekGtkKeyboard*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_gtk_keyboard_new">
				<return-type type="EekKeyboard*"/>
			</constructor>
		</object>
		<constant name="EEK_GTK_H" type="int" value="1"/>
		<constant name="EEK_GTK_KEYBOARD_H" type="int" value="1"/>
	</namespace>
</api>
