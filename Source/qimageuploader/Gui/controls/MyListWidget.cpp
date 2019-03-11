#include "MyListWidget.h"

MyListWidget::MyListWidget(QWidget* parent) :QListWidget(parent) {

}

void MyListWidget::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Delete) {
		auto items = selectedItems();
		for (auto item : items) {
			delete item; // Remove item from list
		}
	}
	QListWidget::keyPressEvent(event);
}

void MyListWidget::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::MiddleButton) {
		auto item = itemAt(event->pos());
		if (item) {
			delete item;
		}
	}
	QListWidget::mousePressEvent(event);
}