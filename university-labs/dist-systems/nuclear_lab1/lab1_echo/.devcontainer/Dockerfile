FROM cr.yandex/crpqcimo58vlheu2317b/ds/nuclear-student:latest

# Install clangd
ADD https://github.com/clangd/clangd/releases/download/18.1.3/clangd-linux-18.1.3.zip /tmp
RUN apt update && apt install -y unzip \
        && cd /tmp \
        && unzip clangd-linux-18.1.3.zip \
        && mv clangd_18.1.3/bin/clangd /usr/bin/clangd \
        && rm -rf clangd-linux-18.1.3.zip clangd_linux-18.1.3

# Configure devcontainer user
ARG REMOTE_USER
ARG REMOTE_UID
ARG REMOTE_GID
#RUN <<EOF
#    addgroup --gid ${REMOTE_GID} ${REMOTE_USER}
#    adduser --disabled-password --uid ${REMOTE_UID} --gid ${REMOTE_GID} ${REMOTE_USER}
#EOF
#ENV HOME /home/${REMOTE_USER}
RUN if id -u ${REMOTE_UID} >/dev/null 2>&1; then \
        EXISTING_USER=$(getent passwd ${REMOTE_UID} | cut -d: -f1); \
        if [ "$EXISTING_USER" != "${REMOTE_USER}" ]; then \
            # Сначала меняем группу существующего пользователя (по старому имени)
            groupmod -g ${REMOTE_GID} ${EXISTING_USER} || true; \
            # Теперь переименовываем пользователя (и его основную группу)
            usermod -l ${REMOTE_USER} $EXISTING_USER; \
            usermod -d /home/${REMOTE_USER} -m ${REMOTE_USER}; \
        else \
            # Пользователь с таким UID и именем уже есть, просто обновляем GID
            groupmod -g ${REMOTE_GID} ${REMOTE_USER} || true; \
        fi; \
    else \
        # UID свободен, создаем с нуля
        groupadd --gid ${REMOTE_GID} ${REMOTE_USER}; \
        adduser --disabled-password --uid ${REMOTE_UID} --gid ${REMOTE_GID} ${REMOTE_USER}; \
    fi

USER ${REMOTE_USER}

