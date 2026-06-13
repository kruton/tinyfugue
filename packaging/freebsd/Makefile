PORTNAME=	tinyfugue
DISTVERSION=	5.0.8
CATEGORIES=	net games

MAINTAINER=	kruton@users.noreply.github.com
COMMENT=	Programmable MUD client
WWW=		https://github.com/kruton/tinyfugue

LICENSE=	GPLv2+
LICENSE_FILE=	${WRKSRC}/COPYING

LIB_DEPENDS=	libicui18n.so:devel/icu \
		libpcre.so:devel/pcre

USES=		cmake compiler:c11 ncurses ssl
USE_GITHUB=	yes
GH_ACCOUNT=	kruton
GH_TAGNAME=	02b048204c5ffd1704b48541724b8ecebc9aee4c

CMAKE_ON=	TF_WIDECHAR TF_TLS TF_TERMCAP TF_ZLIB
CMAKE_OFF=	TF_DEVELOPMENT

do-test:
	cd ${BUILD_WRKSRC} && \
		${SETENV} ${WRK_ENV} ${TEST_ENV} \
		${LOCALBASE}/bin/ctest --output-on-failure

post-install:
	${GZIP_CMD} ${STAGEDIR}${PREFIX}/share/man/man1/tf.1

.include <bsd.port.mk>
