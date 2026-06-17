#include "EduSys/gui/LoginDialog.hpp"

#include <exception>
#include <string>

#include <QDialogButtonBox>
#include <QFrame>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "EduSys/app/AppContext.hpp"
#include "EduSys/common/Exception.hpp"
#include "EduSys/model/UserAccount.hpp"

namespace {

QString errorText(const std::exception& e) {
    return QString::fromLocal8Bit(e.what());
}

class LoginCanvas : public QWidget {
public:
    explicit LoginCanvas(QWidget* parent = nullptr) : QWidget(parent) {
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF area = rect();
        QLinearGradient bg(area.topLeft(), area.bottomRight());
        bg.setColorAt(0.0, QColor(255, 251, 244));
        bg.setColorAt(0.42, QColor(235, 243, 252));
        bg.setColorAt(1.0, QColor(255, 246, 235));
        painter.fillRect(area, bg);

        drawClouds(painter);
        drawStars(painter);
        drawTree(painter);
        drawCity(painter);
        drawHearts(painter);
    }

private:
    void drawClouds(QPainter& painter) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(182, 207, 232, 210));

        const qreal y = height() / 5.0;
        const QList<QPointF> centers = {
            {80.0, y + 18.0}, {150.0, y + 5.0}, {235.0, y + 22.0}, {315.0, y + 8.0}, {410.0, y + 18.0},
            {530.0, y + 3.0}, {645.0, y + 18.0}
        };
        const QList<QSizeF> sizes = {
            {150, 58}, {180, 70}, {160, 62}, {190, 68}, {170, 58}, {155, 58}, {150, 52}
        };
        for (int i = 0; i < centers.size(); ++i) {
            painter.drawEllipse(QRectF(centers[i] - QPointF(sizes[i].width() / 2.0, sizes[i].height() / 2.0), sizes[i]));
        }

        painter.setBrush(QColor(255, 255, 255, 140));
        painter.drawEllipse(QRectF(105, y - 22, 100, 45));
        painter.drawEllipse(QRectF(370, y - 28, 115, 45));
    }

    void drawStars(QPainter& painter) {
        painter.setPen(QPen(QColor(244, 183, 76, 160), 2));
        for (const QPointF& p : {QPointF(605, 55), QPointF(685, 82), QPointF(745, 118),
                                 QPointF(820, 58), QPointF(640, 158)}) {
            painter.drawLine(p + QPointF(-6, 0), p + QPointF(6, 0));
            painter.drawLine(p + QPointF(0, -6), p + QPointF(0, 6));
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(247, 194, 88, 155));
        painter.drawEllipse(QPointF(560, 88), 3, 3);
        painter.drawEllipse(QPointF(710, 36), 3, 3);
        painter.drawEllipse(QPointF(805, 143), 3, 3);
    }

    void drawTree(QPainter& painter) {
        const QPointF trunkBase(width() - 185, height() - 78);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(129, 83, 50));
        QPainterPath trunk;
        trunk.moveTo(trunkBase.x() - 52, trunkBase.y());
        trunk.cubicTo(trunkBase.x() - 37, trunkBase.y() - 120, trunkBase.x() - 30, trunkBase.y() - 215, trunkBase.x() - 10, trunkBase.y() - 270);
        trunk.cubicTo(trunkBase.x() + 20, trunkBase.y() - 220, trunkBase.x() + 28, trunkBase.y() - 130, trunkBase.x() + 36, trunkBase.y());
        trunk.closeSubpath();
        painter.drawPath(trunk);

        painter.setPen(QPen(QColor(104, 62, 39), 5, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(trunkBase + QPointF(-8, -192), trunkBase + QPointF(-84, -248));
        painter.drawLine(trunkBase + QPointF(7, -172), trunkBase + QPointF(75, -226));

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(246, 169, 157, 225));
        const QPointF crown(width() - 245, height() / 2.0 - 20);
        const QList<QPointF> bubbles = {
            {0, 0}, {70, -12}, {130, 18}, {32, 42}, {98, 54}, {-35, 38}, {36, -43}, {104, -42}
        };
        for (const auto& offset : bubbles) {
            painter.drawEllipse(crown + offset, 58, 50);
        }

        painter.setBrush(QColor(255, 210, 205, 110));
        for (const auto& offset : {QPointF(0, -8), QPointF(58, -16), QPointF(105, 20), QPointF(52, 38)}) {
            painter.drawEllipse(crown + offset, 18, 13);
        }

        painter.setPen(QPen(QColor(225, 113, 109), 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QRectF(crown.x() - 60, crown.y() - 55, 210, 130));
    }

    void drawCity(QPainter& painter) {
        const int base = height() - 72;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(191, 207, 218, 130));
        for (int i = 0; i < 11; ++i) {
            const int x = 24 + i * 48;
            const int h = 28 + (i % 4) * 12;
            painter.drawRoundedRect(QRectF(x, base - h, 34, h), 6, 6);
        }

        painter.setPen(QPen(QColor(151, 179, 195, 130), 2));
        painter.drawLine(0, base, width(), base);
    }

    void drawHearts(QPainter& painter) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(235, 83, 91, 170));
        const qreal h = height();
        const QList<QPointF> hearts = {
            {65.0, h - 105.0}, {146.0, h - 92.0}, {245.0, h - 112.0},
            {335.0, h - 88.0}, {520.0, h - 98.0}, {740.0, h - 250.0}
        };
        for (const auto& p : hearts) {
            QPainterPath heart;
            heart.moveTo(p.x(), p.y() + 6);
            heart.cubicTo(p.x() - 13, p.y() - 6, p.x() - 3, p.y() - 18, p.x(), p.y() - 7);
            heart.cubicTo(p.x() + 3, p.y() - 18, p.x() + 13, p.y() - 6, p.x(), p.y() + 6);
            painter.drawPath(heart);
        }
    }
};

} // namespace

namespace EduSys {

LoginDialog::LoginDialog(AppContext& appContext, QWidget* parent)
    : QDialog(parent)
    , appContext_(appContext) {
    setWindowTitle(QString::fromUtf8(u8"登录界面"));
    setModal(true);
    setFixedSize(900, 560);

    auto* dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);

    auto* canvas = new LoginCanvas(this);
    dialogLayout->addWidget(canvas);

    auto* rootLayout = new QVBoxLayout(canvas);
    rootLayout->setContentsMargins(24, 18, 24, 16);
    rootLayout->setSpacing(0);

    auto* titleLabel = new QLabel(QString::fromUtf8(u8"学生成绩管理系统"), canvas);
    titleLabel->setFixedSize(560, 86);
    titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    titleLabel->setStyleSheet(
        "QLabel {"
        "background-color: rgba(47, 47, 47, 245);"
        "color: white;"
        "font-family: 'Microsoft YaHei UI', 'STXingkai';"
        "font-size: 34px;"
        "font-weight: 500;"
        "letter-spacing: 2px;"
        "padding-left: 24px;"
        "}");
    rootLayout->addWidget(titleLabel, 0, Qt::AlignLeft);

    auto* bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(36, 55, 24, 0);
    bodyLayout->setSpacing(12);

    auto* formWrap = new QFrame(canvas);
    formWrap->setObjectName("loginFormWrap");
    formWrap->setFixedWidth(380);
    formWrap->setStyleSheet(
        "#loginFormWrap {"
        "background-color: rgba(255, 255, 255, 75);"
        "border-radius: 16px;"
        "}");
    auto* formRoot = new QVBoxLayout(formWrap);
    formRoot->setContentsMargins(26, 18, 26, 18);
    formRoot->setSpacing(14);

    auto* hintLabel = new QLabel(
        QString::fromUtf8(u8"连续 3 次认证失败后程序会退出。"),
        formWrap);
    hintLabel->setStyleSheet("color: #5f6873; font-size: 13px;");
    formRoot->addWidget(hintLabel);

    auto* formLayout = new QGridLayout();
    formLayout->setHorizontalSpacing(14);
    formLayout->setVerticalSpacing(18);

    usernameEdit_ = new QLineEdit(formWrap);
    passwordEdit_ = new QLineEdit(formWrap);
    passwordEdit_->setEchoMode(QLineEdit::Password);
    usernameEdit_->setPlaceholderText(QString::fromUtf8(u8"请输入用户名"));
    passwordEdit_->setPlaceholderText(QString::fromUtf8(u8"请输入密码"));
    usernameEdit_->setFixedHeight(30);
    passwordEdit_->setFixedHeight(30);
    usernameEdit_->setStyleSheet(
        "QLineEdit {"
        "background-color: #ffffff;"
        "color: #0f172a;"
        "selection-background-color: #bfdbfe;"
        "selection-color: #0f172a;"
        "border: 1px solid #94b7dc;"
        "border-radius: 4px;"
        "padding: 3px 8px;"
        "font-size: 14px;"
        "}"
        "placeholder-text-color: #64748b;"
        "QLineEdit:focus { border: 2px solid #2685d9; }");
    passwordEdit_->setStyleSheet(usernameEdit_->styleSheet());

    auto* accountLabel = new QLabel(QString::fromUtf8(u8"账号："), formWrap);
    auto* passwordLabel = new QLabel(QString::fromUtf8(u8"密码："), formWrap);
    accountLabel->setStyleSheet("font-size: 14px; color: #334155;");
    passwordLabel->setStyleSheet(accountLabel->styleSheet());

    formLayout->addWidget(accountLabel, 0, 0);
    formLayout->addWidget(usernameEdit_, 0, 1);
    formLayout->addWidget(passwordLabel, 1, 0);
    formLayout->addWidget(passwordEdit_, 1, 1);
    formRoot->addLayout(formLayout);

    auto* slogan = new QLabel(QString::fromUtf8(u8"All for you"), formWrap);
    slogan->setAlignment(Qt::AlignCenter);
    slogan->setStyleSheet(
        "font-family: 'Comic Sans MS', 'Segoe Print';"
        "font-size: 28px;"
        "font-weight: 600;"
        "color: #f05f63;");
    formRoot->addWidget(slogan);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(14);
    auto* loginButton = new QPushButton(QString::fromUtf8(u8"登录"), formWrap);
    auto* changePasswordButton = new QPushButton(QString::fromUtf8(u8"修改密码"), formWrap);
    auto* exitButton = new QPushButton(QString::fromUtf8(u8"退出"), formWrap);

    const QString buttonStyle =
        "QPushButton {"
        "background-color: #f8fafc;"
        "color: #1f2937;"
        "border: 1px solid #9aa9ba;"
        "border-radius: 5px;"
        "font-size: 14px;"
        "font-weight: 500;"
        "min-height: 30px;"
        "padding: 4px 16px;"
        "}"
        "QPushButton:hover { background-color: #ffffff; border-color: #2685d9; color: #0f172a; }"
        "QPushButton:pressed { background-color: #dbeafe; color: #0f172a; }"
        "QPushButton:disabled { background-color: #e5e7eb; color: #64748b; border-color: #cbd5e1; }";
    loginButton->setStyleSheet(buttonStyle);
    changePasswordButton->setStyleSheet(buttonStyle);
    exitButton->setStyleSheet(buttonStyle);
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(changePasswordButton);
    buttonLayout->addWidget(exitButton);
    formRoot->addLayout(buttonLayout);

    bodyLayout->addWidget(formWrap, 0, Qt::AlignTop | Qt::AlignLeft);
    bodyLayout->addStretch(1);
    rootLayout->addLayout(bodyLayout, 1);

    auto* footer = new QLabel(QString::fromUtf8(u8"软件作者：夏同"), canvas);
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color: #475569; font-size: 12px;");
    rootLayout->addWidget(footer);

    connect(loginButton, &QPushButton::clicked, this, [this] { tryLogin(); });
    connect(changePasswordButton, &QPushButton::clicked, this, [this] { changePasswordBeforeLogin(); });
    connect(exitButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(usernameEdit_, &QLineEdit::returnPressed, this, [this] {
        passwordEdit_->setFocus();
    });
    connect(passwordEdit_, &QLineEdit::returnPressed, this, [this] { tryLogin(); });

    usernameEdit_->setFocus();
}

void LoginDialog::tryLogin() {
    const QString username = usernameEdit_->text().trimmed();
    const QString password = passwordEdit_->text();

    if (username.isEmpty()) {
        QMessageBox::information(
            this,
            QString::fromUtf8(u8"结束登录"),
            QString::fromUtf8(u8"用户名为空，本次启动将直接退出。"));
        reject();
        return;
    }

    try {
        const UserAccount account =
            appContext_.authService.authenticate(username.toStdString(), password.toStdString());
        session_.login(account.getUsername(), account.getRole(), account.getOwnerId());
        accept();
    } catch (const AuthException& e) {
        ++failedAttempts_;
        QMessageBox::warning(
            this,
            QString::fromUtf8(u8"登录失败"),
            QString::fromUtf8(u8"%1\n\n当前失败次数：%2 / 3")
                .arg(errorText(e))
                .arg(failedAttempts_));

        passwordEdit_->clear();
        passwordEdit_->setFocus();

        if (failedAttempts_ >= 3) {
            QMessageBox::critical(
                this,
                QString::fromUtf8(u8"登录终止"),
                QString::fromUtf8(u8"连续 3 次认证失败，程序将退出。"));
            reject();
        }
    } catch (const EduException& e) {
        QMessageBox::critical(
            this,
            QString::fromUtf8(u8"业务错误"),
            errorText(e));
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this,
            QString::fromUtf8(u8"未知错误"),
            errorText(e));
    }
}

void LoginDialog::changePasswordBeforeLogin() {
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8(u8"修改密码"));
    dialog.setModal(true);
    dialog.setMinimumWidth(390);

    auto* rootLayout = new QVBoxLayout(&dialog);
    auto* formLayout = new QFormLayout();

    auto* usernameEdit = new QLineEdit(&dialog);
    auto* oldPasswordEdit = new QLineEdit(&dialog);
    auto* newPasswordEdit = new QLineEdit(&dialog);
    auto* confirmEdit = new QLineEdit(&dialog);

    usernameEdit->setText(usernameEdit_->text().trimmed());
    oldPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow(QString::fromUtf8(u8"用户名"), usernameEdit);
    formLayout->addRow(QString::fromUtf8(u8"当前密码"), oldPasswordEdit);
    formLayout->addRow(QString::fromUtf8(u8"新密码"), newPasswordEdit);
    formLayout->addRow(QString::fromUtf8(u8"确认新密码"), confirmEdit);
    rootLayout->addLayout(formLayout);

    auto* buttons = new QDialogButtonBox(&dialog);
    auto* okButton = buttons->addButton(QString::fromUtf8(u8"确认修改"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QString::fromUtf8(u8"取消"), QDialogButtonBox::RejectRole);
    rootLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(okButton, &QPushButton::clicked, &dialog, [&] {
        const QString username = usernameEdit->text().trimmed();
        const QString oldPassword = oldPasswordEdit->text();
        const QString newPassword = newPasswordEdit->text();
        const QString confirm = confirmEdit->text();

        if (username.isEmpty()) {
            QMessageBox::warning(&dialog, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"请输入用户名。"));
            usernameEdit->setFocus();
            return;
        }
        if (oldPassword.isEmpty()) {
            QMessageBox::warning(&dialog, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"请输入当前密码。"));
            oldPasswordEdit->setFocus();
            return;
        }
        if (newPassword.isEmpty()) {
            QMessageBox::warning(&dialog, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"请输入新密码。"));
            newPasswordEdit->setFocus();
            return;
        }
        if (newPassword != confirm) {
            QMessageBox::warning(&dialog, QString::fromUtf8(u8"密码不匹配"), QString::fromUtf8(u8"两次输入的新密码不一致。"));
            confirmEdit->setFocus();
            return;
        }

        try {
            const UserAccount account =
                appContext_.authService.authenticate(username.toStdString(), oldPassword.toStdString());
            Session passwordSession;
            passwordSession.login(account.getUsername(), account.getRole(), account.getOwnerId());
            appContext_.authService.changePassword(
                passwordSession,
                oldPassword.toStdString(),
                newPassword.toStdString());
            QMessageBox::information(&dialog, QString::fromUtf8(u8"修改成功"), QString::fromUtf8(u8"密码已更新，请使用新密码登录。"));
            passwordEdit_->clear();
            dialog.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dialog, QString::fromUtf8(u8"修改失败"), errorText(e));
        }
    });

    dialog.exec();
}

} // namespace EduSys
