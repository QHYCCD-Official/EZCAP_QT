#include "accessibleAction.h"

#include <QAction>
#include <QWidget>
#include <QMenu>
#include <QMenuBar>

class ActionAccessible : public QAccessibleInterface, public QAccessibleActionInterface
{
public:
    explicit ActionAccessible(QAction *action)
        : m_action(action) {}

    bool isValid() const { return m_action != nullptr; }
    QObject *object() const { return m_action; }

    QAccessibleInterface *parent() const {
        QObject *par = m_action ? m_action->parent() : nullptr;
        if (!par) return nullptr;
        return QAccessible::queryAccessibleInterface(par);
    }

    QAccessibleInterface *child(int) const { return nullptr; }
    int childCount() const { return 0; }
    int indexOfChild(const QAccessibleInterface *) const { return -1; }

    QString text(QAccessible::Text t) const {
        if (!m_action) return QString();
        switch (t) {
        case QAccessible::Name:
            return m_action->text().remove('&').trimmed();
        case QAccessible::Description:
            if (!m_action->statusTip().isEmpty())
                return m_action->statusTip();
            return m_action->toolTip();
        case QAccessible::Accelerator:
            return m_action->shortcut().toString();
        case QAccessible::Value:
            if (m_action->isCheckable())
                return m_action->isChecked() ? QStringLiteral("1") : QStringLiteral("0");
            return QString();
        default:
            return QString();
        }
    }

    void setText(QAccessible::Text, const QString &) {}

    QRect rect() const {
        if (!m_action) return QRect();
        QWidget *w = qobject_cast<QWidget*>(m_action->parent());
        if (!w) {
            QMenu *m = qobject_cast<QMenu*>(m_action->parent());
            if (m) w = m->parentWidget();
        }
        return w ? w->rect() : QRect();
    }

    QAccessible::Role role() const {
        if (!m_action) return QAccessible::NoRole;
        if (m_action->isSeparator()) return QAccessible::Separator;
        return QAccessible::MenuItem;
    }

    QAccessible::State state() const {
        QAccessible::State st;
        if (!m_action) return st;
        st.disabled = !m_action->isEnabled();
        st.focusable = m_action->isEnabled();
        st.selectable = m_action->isEnabled();
        if (m_action->isChecked()) st.checked = 1;
        return st;
    }

    QAccessibleInterface *focusChild() const { return nullptr; }
    QAccessibleInterface *childAt(int, int) const { return nullptr; }

    void *interface_cast(QAccessible::InterfaceType type) {
        if (type == QAccessible::ActionInterface)
            return static_cast<QAccessibleActionInterface*>(this);
        return nullptr;
    }

    QStringList actionNames() const {
        return QStringList(pressAction());
    }

    void doAction(const QString &actionName) {
        if (actionName == pressAction() && m_action)
            m_action->trigger();
    }

    QStringList keyBindingsForAction(const QString &) const {
        if (m_action && !m_action->shortcut().isEmpty())
            return QStringList(m_action->shortcut().toString());
        return QStringList();
    }

private:
    QAction *m_action;
};

static QAccessibleInterface *actionFactory(const QString &classname, QObject *object)
{
    Q_UNUSED(classname);
    if (QAction *action = qobject_cast<QAction*>(object)) {
        if (action->isSeparator())
            return nullptr;
        return new ActionAccessible(action);
    }
    return nullptr;
}

void setupMenuAccessibility()
{
    QAccessible::installFactory(actionFactory);
}
