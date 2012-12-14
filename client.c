#include "network.h"
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <windows.h>

GtkWidget *window;
GtkObject *adjust;
GtkWidget *playButton;
GtkWidget *stopButton;
GtkWidget *nextButton;
GtkWidget *checkButton;
GtkWidget *leftLabel;
GtkWidget *rightLabel;
GtkWidget *listBox;
GtkWidget *scrollwindow;
GtkWidget *selectButton;
GtkWidget *playlistBox;
GtkWidget *listItem;
double totalSecond;
double second;

void CloseTheApp(GtkWidget *window, gpointer data)
{
	gtk_main_quit();
}

void MsgBox(gchar *data)
{
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *button;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_title(GTK_WINDOW(window), "出错了");
	gtk_widget_set_size_request(GTK_WIDGET(window),200,80);

	gtk_signal_connect(GTK_OBJECT(window),
			"destroy",
			GTK_SIGNAL_FUNC(CloseTheApp),
			NULL);

	table=gtk_table_new(3,3,TRUE);
	label = gtk_label_new(data);
	button = gtk_button_new_with_label("确定");

	gtk_signal_connect(GTK_OBJECT(button),
			"clicked",
			GTK_SIGNAL_FUNC(CloseTheApp),
			NULL);

	gtk_table_attach_defaults(GTK_TABLE(table),label,0,3,0,2);
	gtk_table_attach_defaults(GTK_TABLE(table),button,1,2,2,3);
	gtk_container_add(GTK_CONTAINER(window), table);
	gtk_widget_show_all(window);
}

void AdjustChanged(GtkAdjustment *adjust, gpointer data)
{
	double want_second = totalSecond * ( adjust->value / 10000.0 );
	double diff_second = want_second - second;
	if (diff_second > -0.01 && diff_second < 0.01) {
		return;
	}
	char buff[128] = {0};
	_snprintf(buff, sizeof(buff), "seek %lf", want_second);
	int len = strlen(buff);
	if(SendMsg(buff, len) != len)
	{
		MsgBox("与服务器断开连接");
		return;
	}
}

void ItemSelected(GtkList *list, GtkWidget *widget, gpointer data)
{
	listItem = widget;
	gtk_widget_set_sensitive(selectButton, TRUE);
}

void ItemUnselected(GtkList *list, GtkWidget *widget, gpointer data)
{
	listItem = NULL;
	gtk_widget_set_sensitive(selectButton, FALSE);
}

void SelectMusic(GtkWidget *button, gpointer data)
{
	if (listItem == NULL) 
	{
		return;
	}

	gchar *p;
	gtk_label_get(GTK_LABEL(GTK_BIN(listItem)->child), &p);
	p = g_locale_from_utf8(p, strlen(p), NULL, NULL, NULL);
	char buff[128] = {0};
	_snprintf(buff, sizeof(buff), "select %s", p);
	int len = strlen(buff);
	if (SendMsg(buff, len) != len) 
	{
		MsgBox("与服务器断开连接");
	}
	g_free(p);
}

GtkWidget *AddPlayList()
{
	GtkAdjustment *vertical;
	GtkAdjustment *horizontal;

	vertical = GTK_ADJUSTMENT(gtk_adjustment_new(0,0,0,0,0,0));
	horizontal = GTK_ADJUSTMENT(gtk_adjustment_new(0,0,0,0,0,0));
	scrollwindow = gtk_scrolled_window_new(vertical, horizontal);
	gtk_widget_set_usize(scrollwindow, -1, 150);
	
	listBox = gtk_list_new();
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollwindow), listBox);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_signal_connect(GTK_OBJECT(listBox), "select_child",
			GTK_SIGNAL_FUNC(ItemSelected), NULL);

	gtk_signal_connect(GTK_OBJECT(listBox), "unselect_child",
			GTK_SIGNAL_FUNC(ItemUnselected), NULL);

	gtk_container_add(GTK_CONTAINER(scrollwindow), listBox);

	selectButton = gtk_button_new_with_label("选择");
	gtk_widget_set_sensitive(selectButton, FALSE);

	gtk_signal_connect(GTK_OBJECT(selectButton), "clicked",
			GTK_SIGNAL_FUNC(SelectMusic), NULL);

	playlistBox = gtk_vbox_new(FALSE, 0);

	gtk_box_pack_start(GTK_BOX(playlistBox), scrollwindow, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(playlistBox), selectButton, FALSE, FALSE, 0);

	gtk_widget_show(selectButton);
	gtk_widget_show(listBox);
	gtk_widget_show(scrollwindow);

	return playlistBox;
}

GtkWidget *AddLabelPanel()
{
	GtkWidget *box = gtk_hbox_new(FALSE, 5);
	leftLabel = gtk_label_new("music");
	gtk_misc_set_alignment(GTK_MISC(leftLabel), 0, 0);
	rightLabel = gtk_label_new("time");
	gtk_misc_set_alignment(GTK_MISC(rightLabel), 1, 0);

	gtk_container_add(GTK_CONTAINER(box), leftLabel);
	gtk_container_add(GTK_CONTAINER(box), rightLabel);

	gtk_widget_show(leftLabel);
	gtk_widget_show(rightLabel);
	gtk_widget_show(box);

	return box;
}

void CheckToggled(GtkToggleButton *button, gpointer data)
{
	//if (gtk_widget_get_visible(playlistBox) == TRUE) 
	if (GTK_WIDGET_VISIBLE(playlistBox) == TRUE) 
	{
		gtk_widget_hide(playlistBox);
	}
	else 
	{
		gtk_widget_show(playlistBox);
	}
}

void PlayMusic(GtkToggleButton *button, gpointer data)
{
	const gchar *pLabel = gtk_button_get_label(GTK_BUTTON(button));

	if (strcmp(pLabel, "播放") == 0) 
	{
		if(SendMsg("play", 4) != 4)
		{
			MsgBox("与服务器断开连接");
			return;
		}
		gtk_button_set_label(GTK_BUTTON(button), "暂停");
	}
	else 
	{
		if(SendMsg("pause", 5) != 5)
		{
			MsgBox("与服务器断开连接");
			return;
		}
		gtk_button_set_label(GTK_BUTTON(button), "播放");
	}
}

void StopMusic(GtkToggleButton *button, gpointer data)
{
	if(SendMsg("stop", 4) != 4)
	{
		MsgBox("与服务器断开连接");
		return;
	}
	gtk_button_set_label(GTK_BUTTON(playButton), "播放");
}

void NextMusic(GtkToggleButton *button, gpointer data)
{
	if(SendMsg("next", 4) != 4)
	{
		MsgBox("与服务器断开连接");
		return;
	}
	gtk_button_set_label(GTK_BUTTON(playButton), "播放");
}

GtkWidget *AddControlPanel()
{
	GtkWidget *box;

	box = gtk_hbox_new(TRUE, 5);

	playButton = gtk_button_new_with_label("播放");
	stopButton = gtk_button_new_with_label("停止");
	nextButton = gtk_button_new_with_label("下一首");
	checkButton = gtk_check_button_new_with_label("播放列表");

	gtk_container_add(GTK_CONTAINER(box), playButton);
	gtk_container_add(GTK_CONTAINER(box), stopButton);
	gtk_container_add(GTK_CONTAINER(box), nextButton);
	gtk_container_add(GTK_CONTAINER(box), checkButton);

	gtk_signal_connect(GTK_OBJECT(nextButton),
			"clicked",
			GTK_SIGNAL_FUNC(NextMusic),
			NULL);
	gtk_signal_connect(GTK_OBJECT(playButton),
			"clicked",
			GTK_SIGNAL_FUNC(PlayMusic),
			NULL);
	gtk_signal_connect(GTK_OBJECT(stopButton),
			"clicked",
			GTK_SIGNAL_FUNC(StopMusic),
			NULL);
	gtk_signal_connect(GTK_OBJECT(checkButton),
			"toggled",
			GTK_SIGNAL_FUNC(CheckToggled),
			NULL);

	gtk_widget_show(playButton);
	gtk_widget_show(stopButton);
	gtk_widget_show(nextButton);
	gtk_widget_show(checkButton);
	gtk_widget_show(box);

	return box;
}

GtkWidget *BuildMainVBox()
{
	GtkWidget *box;
	GtkWidget *scale;

	box = gtk_vbox_new(FALSE, 5);

	adjust = GTK_OBJECT(gtk_adjustment_new(0,0,10000,1,1,1));
	gtk_signal_connect(adjust,
			"value_changed",
			GTK_SIGNAL_FUNC(AdjustChanged),
			NULL);

	scale = gtk_hscale_new(GTK_ADJUSTMENT(adjust));
	gtk_scale_set_digits(GTK_SCALE(scale), 0);
	gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);

	gtk_container_add(GTK_CONTAINER(box), AddLabelPanel());
	gtk_container_add(GTK_CONTAINER(box), scale);
	gtk_container_add(GTK_CONTAINER(box), AddControlPanel());
	gtk_container_add(GTK_CONTAINER(box), AddPlayList());

	gtk_widget_show(scale);
	gtk_widget_show(box);
	
	return box;
}

void ShowMainWindow()
{
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_signal_connect(GTK_OBJECT(window),
			"destroy",
			GTK_SIGNAL_FUNC(CloseTheApp),
			NULL);

	gtk_container_add(GTK_CONTAINER(window), BuildMainVBox());

	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_title(GTK_WINDOW(window), "Mp3播放器");

	gtk_widget_show(window);
}

gint GetStatus(gpointer data)
{
	int ret = SendMsg("current", 7);
	if (ret != 7) 
	{
		MsgBox("与服务器断开连接");
		return FALSE;
	}

	char buff[128] = {0};
	ret = RecvMsg(buff, sizeof(buff));
	if (ret <= 0) 
	{
		MsgBox("与服务器断开连接");
		return FALSE;
	}

	double second_left;

	char *file;

	if (strncmp(buff, "stop", 4) == 0)
	{
		gtk_button_set_label(GTK_BUTTON(playButton), "播放");
		gtk_widget_set_sensitive(stopButton, FALSE);
		file = &buff[4];
	}
	else if (strncmp(buff, "pause", 5) == 0) 
	{
		gtk_button_set_label(GTK_BUTTON(playButton), "播放");
		gtk_widget_set_sensitive(stopButton, FALSE);
		file = &buff[5];
	}
	else
	{
		gtk_button_set_label(GTK_BUTTON(playButton), "暂停");
		gtk_widget_set_sensitive(stopButton, TRUE);
		file = &buff[4];
	}

	*file = '\0';
	file++;

	char *p = strrchr(file, ' ');
	second_left = atof(++p);
	p--;
	*p = '\0';
	p = strrchr(file, ' ');
	second = atof(++p);
	p--;
	*p = '\0';

	totalSecond = second + second_left;
	char time_buff[128] = {0};
	_snprintf(time_buff, sizeof(time_buff), "%.1lf/%.1lf", second, totalSecond);

	gchar *q = g_locale_to_utf8(file, strlen(file), NULL, NULL, NULL);
	gtk_label_set_text(GTK_LABEL(leftLabel), q);
	gtk_label_set_text(GTK_LABEL(rightLabel), time_buff);

	gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), second / totalSecond * 10000.0);

	gtk_window_set_title(GTK_WINDOW(window), q);
	g_free(q);
	return TRUE;
}

void AddToPlayList(gchar *p)
{
	gchar *q = g_locale_to_utf8(p, strlen(p), NULL, NULL, NULL);
	GtkWidget *item = gtk_list_item_new_with_label(q);
	gtk_container_add(GTK_CONTAINER(listBox), item);
	gtk_widget_show(item);
	g_free(q);
}

gint GetPlayList()
{
	if(SendMsg("list", 4) != 4)
	{
		MsgBox("与服务器断开连接");
		return FALSE;
	}

	char buff[10240] = {0};
	int ret = RecvMsg(buff, sizeof(buff));
	if (ret <= 0) 
	{
		MsgBox("与服务器断开连接");
		return FALSE;
	}

	char *p = buff;
	char *q;
	while(TRUE)
	{
		q = strstr(p, "\n");
		if (q == NULL) 
		{
			break;
		}

		*q = '\0';
		AddToPlayList(p);
		p = q + 1;
	}

	return TRUE;
}

gint main(gint argc, gchar *argv[])
{
	gtk_init(&argc, &argv);

	if (ConnectToServer("127.0.0.1", 6600) == 0) 
	{
		ShowMainWindow();
	}
	else
	{
		MsgBox("服务器未开启或者有另一个客户端在运行");
	}

	if( GetStatus(NULL) == TRUE
			&& GetPlayList() == TRUE)
	{
		g_timeout_add(300, GetStatus, NULL);
	}

	gtk_main();

	return 0;
}
