<?xml version="1.0"?>
<api version="1.0">
	<namespace name="EekClutter">
		<object name="EekClutterDrawingContext" parent="GInitiallyUnowned" type-name="EekClutterDrawingContext" get-type="eek_clutter_drawing_context_get_type">
			<method name="get_category_font" symbol="eek_clutter_drawing_context_get_category_font">
				<return-type type="PangoFontDescription*"/>
				<parameters>
					<parameter name="context" type="EekClutterDrawingContext*"/>
					<parameter name="category" type="EekKeysymCategory"/>
				</parameters>
			</method>
			<method name="get_outline_texture" symbol="eek_clutter_drawing_context_get_outline_texture">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="context" type="EekClutterDrawingContext*"/>
					<parameter name="outline" type="EekOutline*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_clutter_drawing_context_new">
				<return-type type="EekClutterDrawingContext*"/>
			</constructor>
			<method name="set_category_font" symbol="eek_clutter_drawing_context_set_category_font">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekClutterDrawingContext*"/>
					<parameter name="category" type="EekKeysymCategory"/>
					<parameter name="fonts" type="PangoFontDescription*"/>
				</parameters>
			</method>
			<method name="set_outline_texture" symbol="eek_clutter_drawing_context_set_outline_texture">
				<return-type type="void"/>
				<parameters>
					<parameter name="context" type="EekClutterDrawingContext*"/>
					<parameter name="outline" type="EekOutline*"/>
					<parameter name="texture" type="ClutterActor*"/>
				</parameters>
			</method>
		</object>
		<object name="EekClutterKey" parent="EekKey" type-name="EekClutterKey" get-type="eek_clutter_key_get_type">
			<method name="get_actor" symbol="eek_clutter_key_get_actor">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="key" type="EekClutterKey*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_clutter_key_new">
				<return-type type="EekKey*"/>
				<parameters>
					<parameter name="context" type="EekClutterDrawingContext*"/>
					<parameter name="column" type="gint"/>
					<parameter name="row" type="gint"/>
				</parameters>
			</constructor>
		</object>
		<object name="EekClutterKeyboard" parent="EekKeyboard" type-name="EekClutterKeyboard" get-type="eek_clutter_keyboard_get_type">
			<method name="get_actor" symbol="eek_clutter_keyboard_get_actor">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="keyboard" type="EekClutterKeyboard*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_clutter_keyboard_new">
				<return-type type="EekKeyboard*"/>
			</constructor>
		</object>
		<object name="EekClutterSection" parent="EekSection" type-name="EekClutterSection" get-type="eek_clutter_section_get_type">
			<method name="get_actor" symbol="eek_clutter_section_get_actor">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="section" type="EekClutterSection*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="eek_clutter_section_new">
				<return-type type="EekSection*"/>
				<parameters>
					<parameter name="context" type="EekClutterDrawingContext*"/>
				</parameters>
			</constructor>
		</object>
		<constant name="EEK_CLUTTER_DRAWING_CONTEXT_H" type="int" value="1"/>
		<constant name="EEK_CLUTTER_KEYBOARD_H" type="int" value="1"/>
		<constant name="EEK_CLUTTER_KEY_H" type="int" value="1"/>
		<constant name="EEK_CLUTTER_SECTION_H" type="int" value="1"/>
	</namespace>
</api>
