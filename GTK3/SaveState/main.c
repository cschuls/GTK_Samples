/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+
#+     Gtk Sample Programs
#+
#+     Copyright (C) 2022 by Craig Schulstad
#+
#+
#+ This program is free software; you can redistribute it and/or modify
#+ it under the terms of the GNU General Public License as published by
#+ the Free Software Foundation; either version 2 of the License, or
#+ (at your option) any later version.
#+
#+ This program is distributed in the hope that it will be useful,
#+ but WITHOUT ANY WARRANTY; without even the implied warranty of
#+ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#+ GNU General Public License for more details.
#+
#+ You should have received a copy of the GNU General Public License
#+ along with this program; if not, write to the Free Software
#+ Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#+
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#+
#* This is a sample program to illustrate a method for saving the
#* persistant state of a widget in an XML file.
#*
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

/* The following command will create the executable program in the same folder as the source file on a Linux system using the GCC compiler. */

/* gcc -o SaveState main.c `pkg-config gtk+-3.0 --cflags` `xml2-config --cflags` `pkg-config gtk+-3.0 --libs` `xml2-config --libs`          */

static gboolean result;                                 /* This boolean flag was made global as it is updated via a recursive function. */

void get_xml_state(xmlNode * node, int indent_len)
{
    while(node)
    {
        if(node->type == XML_ELEMENT_NODE)
        {
            if (strcmp("onoff", (char*)node->name) == 0)
            {
                if (strcmp("ON", (char*)xmlNodeGetContent(node)) == 0)
                    result = TRUE;
                else
                    result = FALSE;
            }
        }
        get_xml_state(node->children, indent_len + 1);
        node = node->next;
    }
}

void read_xml_file(GtkToggleButton *button1, GtkToggleButton *button2)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    doc = xmlReadFile("toggle_state.xml", NULL, 0);

    if (doc == NULL)
    {
        printf("Could not parse the XML file");
    }

    root_element = xmlDocGetRootElement(doc);

    get_xml_state(root_element, 1);


    if (result == TRUE)
    {
        gtk_toggle_button_set_active(button1, TRUE);
        gtk_toggle_button_set_active(button2, FALSE);
    }
    else
    {
        gtk_toggle_button_set_active(button1, FALSE);
        gtk_toggle_button_set_active(button2, TRUE);
    }

    xmlFreeDoc(doc);

    xmlCleanupParser();
}

void write_xml_file(gboolean status)
{
    xmlTextWriterPtr writer;

    writer = xmlNewTextWriterFilename("toggle_state.xml", 0);

    xmlTextWriterSetIndent(writer, 4);
    xmlTextWriterSetIndentString(writer, (xmlChar*)"    ");
    xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
    xmlTextWriterStartElement(writer, (xmlChar *) "heartbeat");
    xmlTextWriterStartElement(writer, (xmlChar *) "status");

    if (status)
        xmlTextWriterWriteElement(writer, (xmlChar *) "onoff", (xmlChar *) "ON");
    else
        xmlTextWriterWriteElement(writer, (xmlChar *) "onoff", (xmlChar *) "OFF");

    xmlTextWriterEndElement(writer);
    xmlTextWriterEndElement(writer);
    xmlTextWriterEndElement(writer);
    xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);
}

gboolean on_toggle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == TRUE)
    {
        return TRUE;
    }

    return FALSE;
}

void on_run_toggle_active(GObject *obj, GParamSpec *pspec, gpointer user_data)
{
    g_return_if_fail (user_data != NULL);

    GtkLabel *label = GTK_LABEL(user_data);
    GtkToggleButton *button = GTK_TOGGLE_BUTTON(obj);

    if (gtk_toggle_button_get_active(button) == TRUE)
    {
        gtk_label_set_markup (GTK_LABEL (label), "Running..");
        write_xml_file(TRUE);
    }
    else
    {
        gtk_label_set_text (label, "Idle..");
        write_xml_file(FALSE);
    }
}

gint main(gint argc, gchar **argv)
{
    GtkLabel        *status;
    GtkWindow       *window;
    GtkBuilder      *builder;
    GtkToggleButton *run_toggle;
    GtkToggleButton *kill_toggle;

    gtk_init(&argc, &argv);

    builder         = gtk_builder_new_from_file("main_glade.glade");

    window          = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    status          = GTK_LABEL(gtk_builder_get_object(builder, "status"));
    run_toggle      = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "toggle"));
    kill_toggle     = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "kill"));

    g_object_bind_property (G_OBJECT(run_toggle), "active", G_OBJECT(kill_toggle), "active", G_BINDING_INVERT_BOOLEAN);
    g_object_bind_property (G_OBJECT(kill_toggle), "active", G_OBJECT(run_toggle), "active", G_BINDING_INVERT_BOOLEAN);

    g_signal_connect(G_OBJECT(run_toggle), "button-press-event", G_CALLBACK(on_toggle_button_press_event), NULL);
    g_signal_connect(G_OBJECT(kill_toggle), "button-press-event", G_CALLBACK(on_toggle_button_press_event), NULL);
    g_signal_connect(G_OBJECT(run_toggle), "notify::active", G_CALLBACK(on_run_toggle_active), status);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_window_set_default_size(GTK_WINDOW(window), 360, 200);
    gtk_window_set_title(GTK_WINDOW(window), "Save State");

    read_xml_file(run_toggle, kill_toggle);

    /* The initial label text is slightly different so as to note that it is being set up from the XML data. */

    if (result == TRUE)                             /* "if (result)" would also work since "result" is a boolean variable. */
        gtk_label_set_text (status, "<<Running>>");
    else
        gtk_label_set_text (status, "<<Idle>>");

    gtk_widget_show_all(GTK_WIDGET(window));

    gtk_main();

    return 0;
}
