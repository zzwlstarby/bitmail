#include "certdialog.h"
#include "ui_certdialog.h"
#include "main.h"

#include <QLineEdit>
#include <QLabel>
#include <QPlainTextEdit>
#include <bitmailcore/bitmail.h>

CertDialog::CertDialog(BitMail * bm, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertDialog),
    m_bitmail(bm)
{
    ui->setupUi(this);

    m_leEmail  = findChild<QLineEdit*>("leEmail"  );
    m_leNick   = findChild<QLineEdit*>("leNick"   );
    m_leCertID = findChild<QLineEdit*>("leCertID" );
    m_lbQrCode = findChild<QLabel*>   ("lbQrCode" );
    m_leComment= findChild<QLineEdit*>("leComment");

    m_leEmail->setReadOnly(true);
    m_leNick->setReadOnly(true);
    m_leCertID->setReadOnly(true);
}

CertDialog::~CertDialog()
{
    delete ui;
}

void CertDialog::on_buttonBox_accepted()
{

}

void CertDialog::on_buttonBox_rejected()
{

}

void CertDialog::SetEmail(const QString & e)
{
    m_leEmail->setText(e);
}

QString CertDialog::GetEmail() const
{
    return m_leEmail->text();
}

void CertDialog::SetNick(const QString & n)
{
    m_leNick->setText(n);
}

QString CertDialog::GetNick() const
{
    return m_leNick->text();
}

void CertDialog::CertDialog::SetCertID(const QString & cid)
{
    m_leCertID->setText(cid);
}

QString CertDialog::CertDialog::GetCertID() const
{
    return m_leCertID->text();
}

void CertDialog::comment(const QString &cmt)
{
    m_leComment->setText(cmt);
}

QString CertDialog::comment() const
{
    return m_leComment->text();
}

void CertDialog::qrImage(const QPixmap &p)
{
    m_lbQrCode->setScaledContents(true);
    m_lbQrCode->setPixmap(p);
}

void CertDialog::on_btnSetComment_clicked()
{
    if (!GetEmail().isEmpty() && !comment().isEmpty()){
        std::string sessKey = BitMail::toSessionKey(GetEmail().toStdString());
        m_bitmail->sessionName(sessKey, comment().toStdString());
    }
    emit friendChanged();
}
